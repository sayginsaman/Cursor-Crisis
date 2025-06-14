const supabase = require('../config/supabase');

class SkillService {
  async getAllSkills() {
    const { data, error } = await supabase
      .from('skills')
      .select(`
        *,
        skill_effects (*),
        skill_prerequisites (
          required_level,
          required_skill:required_skill_id (
            skill_id,
            name
          )
        )
      `)
      .order('category', { ascending: true })
      .order('name', { ascending: true });

    if (error) {
      throw new Error(error.message);
    }

    return data;
  }

  async getSkillsByCategory() {
    const skills = await this.getAllSkills();
    
    const categories = {};
    skills.forEach(skill => {
      if (!categories[skill.category]) {
        categories[skill.category] = [];
      }
      categories[skill.category].push(skill);
    });

    return categories;
  }

  async getSkillBySkillId(skillId) {
    const { data, error } = await supabase
      .from('skills')
      .select(`
        *,
        skill_effects (*),
        skill_prerequisites (
          required_level,
          required_skill:required_skill_id (
            skill_id,
            name
          )
        )
      `)
      .eq('skill_id', skillId)
      .single();

    if (error && error.code !== 'PGRST116') {
      throw new Error(error.message);
    }

    return data;
  }

  async getUserSkills(userId) {
    const { data, error } = await supabase
      .from('user_skills')
      .select(`
        level,
        experience,
        unlocked_at,
        skills (
          id,
          skill_id,
          name,
          description,
          category,
          icon,
          max_level,
          base_cost,
          cost_multiplier,
          skill_effects (*),
          skill_prerequisites (
            required_level,
            required_skill:required_skill_id (
              skill_id,
              name
            )
          )
        )
      `)
      .eq('user_id', userId)
      .order('skills(category)', { ascending: true });

    if (error) {
      throw new Error(error.message);
    }

    return data;
  }

  async getUserSkillsWithAvailableUpgrades(userId) {
    // Get all skills and user's current skills
    const [allSkills, userSkills, userStats] = await Promise.all([
      this.getAllSkills(),
      this.getUserSkills(userId),
      supabase.from('users').select('skill_points, level').eq('id', userId).single()
    ]);

    if (userStats.error) {
      throw new Error(userStats.error.message);
    }

    const userSkillMap = new Map();
    userSkills.forEach(us => {
      userSkillMap.set(us.skills.skill_id, us.level);
    });

    // Add upgrade availability information
    const skillsWithUpgradeInfo = allSkills.map(skill => {
      const currentLevel = userSkillMap.get(skill.skill_id) || 0;
      const canUpgrade = this.canUpgradeSkill(skill, currentLevel, userStats.data.skill_points, userStats.data.level, userSkillMap);
      const upgradeCost = this.calculateUpgradeCost(skill, currentLevel);

      return {
        ...skill,
        currentLevel,
        canUpgrade,
        upgradeCost,
        isMaxLevel: currentLevel >= skill.max_level
      };
    });

    return {
      skills: skillsWithUpgradeInfo,
      userStats: userStats.data
    };
  }

  canUpgradeSkill(skill, currentLevel, userSkillPoints, userLevel, userSkillMap) {
    // Check if already at max level
    if (currentLevel >= skill.max_level) {
      return false;
    }

    // Check if user has enough skill points
    const upgradeCost = this.calculateUpgradeCost(skill, currentLevel);
    if (userSkillPoints < upgradeCost) {
      return false;
    }

    // Check user level requirement
    if (userLevel < skill.unlock_level) {
      return false;
    }

    // Check prerequisites
    if (skill.skill_prerequisites && skill.skill_prerequisites.length > 0) {
      for (const prereq of skill.skill_prerequisites) {
        const requiredSkillLevel = userSkillMap.get(prereq.required_skill.skill_id) || 0;
        if (requiredSkillLevel < prereq.required_level) {
          return false;
        }
      }
    }

    return true;
  }

  calculateUpgradeCost(skill, currentLevel) {
    const nextLevel = currentLevel + 1;
    return Math.floor(skill.base_cost * Math.pow(skill.cost_multiplier, nextLevel - 1));
  }

  async upgradeSkill(userId, skillId) {
    // Start a transaction-like operation
    const skill = await this.getSkillBySkillId(skillId);
    if (!skill) {
      throw new Error('Skill not found');
    }

    // Get user's current skill level and stats
    const [userSkillResult, userStatsResult] = await Promise.all([
      supabase
        .from('user_skills')
        .select('level')
        .eq('user_id', userId)
        .eq('skill_id', skill.id)
        .single(),
      supabase
        .from('users')
        .select('skill_points, level')
        .eq('id', userId)
        .single()
    ]);

    const currentLevel = userSkillResult.data?.level || 0;
    const userStats = userStatsResult.data;

    if (!userStats) {
      throw new Error('User not found');
    }

    // Get all user skills for prerequisite checking
    const allUserSkills = await this.getUserSkills(userId);
    const userSkillMap = new Map();
    allUserSkills.forEach(us => {
      userSkillMap.set(us.skills.skill_id, us.level);
    });

    // Check if upgrade is possible
    if (!this.canUpgradeSkill(skill, currentLevel, userStats.skill_points, userStats.level, userSkillMap)) {
      throw new Error('Cannot upgrade this skill');
    }

    const upgradeCost = this.calculateUpgradeCost(skill, currentLevel);
    const newLevel = currentLevel + 1;

    // Update or insert user skill
    let skillUpdateResult;
    if (currentLevel === 0) {
      // First time learning this skill
      skillUpdateResult = await supabase
        .from('user_skills')
        .insert([{
          user_id: userId,
          skill_id: skill.id,
          level: newLevel
        }])
        .select()
        .single();
    } else {
      // Upgrading existing skill
      skillUpdateResult = await supabase
        .from('user_skills')
        .update({ level: newLevel })
        .eq('user_id', userId)
        .eq('skill_id', skill.id)
        .select()
        .single();
    }

    if (skillUpdateResult.error) {
      throw new Error(skillUpdateResult.error.message);
    }

    // Deduct skill points
    const userUpdateResult = await supabase
      .from('users')
      .update({ skill_points: userStats.skill_points - upgradeCost })
      .eq('id', userId)
      .select('skill_points')
      .single();

    if (userUpdateResult.error) {
      throw new Error(userUpdateResult.error.message);
    }

    return {
      skill: {
        ...skill,
        level: newLevel
      },
      remainingSkillPoints: userUpdateResult.data.skill_points,
      upgradeCost
    };
  }

  async createSkill(skillData) {
    const { effects = [], prerequisites = [], ...skillInfo } = skillData;

    // Insert skill
    const { data: skill, error: skillError } = await supabase
      .from('skills')
      .insert([skillInfo])
      .select()
      .single();

    if (skillError) {
      throw new Error(skillError.message);
    }

    // Insert effects
    if (effects.length > 0) {
      const effectsWithSkillId = effects.map(effect => ({
        ...effect,
        skill_id: skill.id
      }));

      const { error: effectsError } = await supabase
        .from('skill_effects')
        .insert(effectsWithSkillId);

      if (effectsError) {
        throw new Error(effectsError.message);
      }
    }

    // Insert prerequisites
    if (prerequisites.length > 0) {
      const prereqsWithSkillId = prerequisites.map(prereq => ({
        ...prereq,
        skill_id: skill.id
      }));

      const { error: prereqsError } = await supabase
        .from('skill_prerequisites')
        .insert(prereqsWithSkillId);

      if (prereqsError) {
        throw new Error(prereqsError.message);
      }
    }

    return await this.getSkillBySkillId(skill.skill_id);
  }
}

module.exports = new SkillService(); 