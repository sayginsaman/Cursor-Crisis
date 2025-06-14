const express = require('express');
const { body, validationResult } = require('express-validator');
const skillService = require('../services/skillService');
const userService = require('../services/userService');
const { authMiddleware } = require('../middleware/auth');

const router = express.Router();

// @route   GET /api/skills
// @desc    Get all available skills
// @access  Public
router.get('/', async (req, res, next) => {
  try {
    const skills = await skillService.getAllSkills();

    res.json({
      success: true,
      data: {
        skills: skills.map(skill => ({
          skillId: skill.skill_id,
          name: skill.name,
          description: skill.description,
          category: skill.category,
          icon: skill.icon,
          maxLevel: skill.max_level,
          baseCost: skill.base_cost,
          costMultiplier: skill.cost_multiplier,
          effects: skill.skill_effects,
          prerequisites: skill.skill_prerequisites,
          unlockLevel: skill.unlock_level
        }))
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/skills/user
// @desc    Get user's skills with progress
// @access  Private
router.get('/user', authMiddleware, async (req, res, next) => {
  try {
    const result = await skillService.getUserSkillsWithAvailableUpgrades(req.user.id);

    const userSkillsWithProgress = result.skills.map(skill => ({
      skillId: skill.skill_id,
      name: skill.name,
      description: skill.description,
      category: skill.category,
      icon: skill.icon,
      maxLevel: skill.max_level,
      currentLevel: skill.currentLevel,
      isUnlocked: result.userStats.level >= skill.unlock_level,
      prerequisitesMet: skill.canUpgrade || skill.currentLevel > 0,
      canUpgrade: skill.canUpgrade,
      nextLevelCost: skill.upgradeCost,
      effects: skill.skill_effects.map(effect => ({
        type: effect.effect_type,
        currentValue: skill.currentLevel > 0 ? 
          (effect.base_value + (skill.currentLevel * effect.per_level_increase)) : 0,
        nextValue: skill.currentLevel < skill.max_level ? 
          (effect.base_value + ((skill.currentLevel + 1) * effect.per_level_increase)) : 0,
        isPercentage: effect.is_percentage
      })),
      prerequisites: skill.skill_prerequisites.map(prereq => ({
        skillId: prereq.required_skill.skill_id,
        level: prereq.required_level
      })),
      unlockLevel: skill.unlock_level,
      isMaxLevel: skill.isMaxLevel
    }));

    res.json({
      success: true,
      data: {
        skills: userSkillsWithProgress,
        currency: {
          skillPoints: result.userStats.skill_points,
          coins: result.userStats.coins
        },
        userLevel: result.userStats.level
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   POST /api/skills/upgrade
// @desc    Upgrade a skill
// @access  Private
router.post('/upgrade', [
  body('skillId')
    .notEmpty()
    .withMessage('Skill ID is required')
], authMiddleware, async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { skillId } = req.body;
    
    try {
      const result = await skillService.upgradeSkill(req.user.id, skillId);

      res.json({
        success: true,
        data: {
          skill: {
            skillId: result.skill.skill_id,
            newLevel: result.skill.level,
            costPaid: result.upgradeCost,
            effects: result.skill.skill_effects.map(effect => ({
              type: effect.effect_type,
              newValue: effect.base_value + (result.skill.level * effect.per_level_increase),
              isPercentage: effect.is_percentage
            }))
          },
          remainingSkillPoints: result.remainingSkillPoints
        }
      });
    } catch (upgradeError) {
      return res.status(400).json({
        success: false,
        error: upgradeError.message
      });
    }
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/skills/categories
// @desc    Get skills grouped by category
// @access  Public
router.get('/categories', async (req, res, next) => {
  try {
    const categories = await skillService.getSkillsByCategory();

    const formattedCategories = {};
    for (const [category, skills] of Object.entries(categories)) {
      formattedCategories[category] = skills.map(skill => ({
        skillId: skill.skill_id,
        name: skill.name,
        description: skill.description,
        icon: skill.icon,
        maxLevel: skill.max_level,
        baseCost: skill.base_cost,
        costMultiplier: skill.cost_multiplier,
        effects: skill.skill_effects,
        prerequisites: skill.skill_prerequisites,
        unlockLevel: skill.unlock_level
      }));
    }

    res.json({
      success: true,
      data: {
        categories: formattedCategories
      }
    });
  } catch (error) {
    next(error);
  }
});

module.exports = router; 