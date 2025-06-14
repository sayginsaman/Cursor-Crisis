const supabase = require('../config/supabase');

class ScoreService {
  constructor() {}

  // Save Normal Score (individual game session score)
  async saveNormalScore(profileId, sessionId, scoreData) {
    const {
      score,
      survivalTime,
      kills,
      enemiesSpawned,
      damageDealt,
      damageTaken,
      waveReached
    } = scoreData;

    try {
      // Check if this is a personal best
      const { data: currentBest } = await supabase
        .from('profiles')
        .select('highest_score')
        .eq('id', profileId)
        .single();

      const isPersonalBest = !currentBest || score > (currentBest.highest_score || 0);

      // Insert normal score record
      const { data: normalScore, error: normalScoreError } = await supabase
        .from('normal_scores')
        .insert([{
          profile_id: profileId,
          session_id: sessionId,
          score: score,
          survival_time: survivalTime,
          kills: kills || 0,
          enemies_spawned: enemiesSpawned || 0,
          damage_dealt: damageDealt || 0,
          damage_taken: damageTaken || 0,
          wave_reached: waveReached || 1,
          is_personal_best: isPersonalBest
        }])
        .select()
        .single();

      if (normalScoreError) {
        throw new Error(`Failed to save normal score: ${normalScoreError.message}`);
      }

      // Update profile highest score if needed
      if (isPersonalBest) {
        await supabase
          .from('profiles')
          .update({ highest_score: score })
          .eq('id', profileId);
      }

      return {
        success: true,
        normalScore,
        isPersonalBest
      };
    } catch (error) {
      console.error('Error saving normal score:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Save Leaderboard Score (accumulated leaderboard points)
  async saveLeaderboardScore(profileId, sessionId, leaderboardData) {
    const {
      pointsEarnedThisSession,
      survivalTime,
      sessionStartTime,
      sessionEndTime
    } = leaderboardData;

    try {
      // Get current total leaderboard points
      const { data: currentProfile } = await supabase
        .from('profiles')
        .select('current_leaderboard_points')
        .eq('id', profileId)
        .single();

      const currentTotal = currentProfile?.current_leaderboard_points || 0;
      const newTotal = currentTotal + pointsEarnedThisSession;

      // Insert leaderboard score record
      const { data: leaderboardScore, error: leaderboardError } = await supabase
        .from('leaderboard_scores')
        .insert([{
          profile_id: profileId,
          session_id: sessionId,
          leaderboard_points: pointsEarnedThisSession,
          points_earned_this_session: pointsEarnedThisSession,
          total_accumulated_points: newTotal,
          survival_time: survivalTime,
          earning_rate: 2.0, // 2 points per second
          session_start_time: sessionStartTime,
          session_end_time: sessionEndTime
        }])
        .select()
        .single();

      if (leaderboardError) {
        throw new Error(`Failed to save leaderboard score: ${leaderboardError.message}`);
      }

      // Update profile current leaderboard points
      await supabase
        .from('profiles')
        .update({ current_leaderboard_points: newTotal })
        .eq('id', profileId);

      return {
        success: true,
        leaderboardScore,
        newTotal
      };
    } catch (error) {
      console.error('Error saving leaderboard score:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Save Skill Score (skill points for purchasing upgrades)
  async saveSkillScore(profileId, sessionId, skillData) {
    const {
      pointsEarnedThisSession,
      survivalTime,
      sessionStartTime,
      sessionEndTime
    } = skillData;

    try {
      // Get current total skill points
      const { data: currentProfile } = await supabase
        .from('profiles')
        .select('current_skill_points')
        .eq('id', profileId)
        .single();

      const currentTotal = currentProfile?.current_skill_points || 0;
      const newTotal = currentTotal + pointsEarnedThisSession;

      // Insert skill score record
      const { data: skillScore, error: skillError } = await supabase
        .from('skill_scores')
        .insert([{
          profile_id: profileId,
          session_id: sessionId,
          skill_points: pointsEarnedThisSession,
          points_earned_this_session: pointsEarnedThisSession,
          total_accumulated_points: newTotal,
          points_spent: 0,
          points_available: newTotal,
          survival_time: survivalTime,
          earning_rate: 1.0, // 1 point per second
          session_start_time: sessionStartTime,
          session_end_time: sessionEndTime
        }])
        .select()
        .single();

      if (skillError) {
        throw new Error(`Failed to save skill score: ${skillError.message}`);
      }

      // Update profile current skill points
      await supabase
        .from('profiles')
        .update({ current_skill_points: newTotal })
        .eq('id', profileId);

      return {
        success: true,
        skillScore,
        newTotal
      };
    } catch (error) {
      console.error('Error saving skill score:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Save all scores at once (called when game session ends)
  async saveAllScores(profileId, sessionId, allScoreData) {
    const {
      normalScore,
      leaderboardScore,
      skillScore,
      sessionStartTime,
      sessionEndTime
    } = allScoreData;

    try {
      console.log(`Saving all scores for profile ${profileId}, session ${sessionId}`);

      // Save all three score types
      const [normalResult, leaderboardResult, skillResult] = await Promise.all([
        this.saveNormalScore(profileId, sessionId, normalScore),
        this.saveLeaderboardScore(profileId, sessionId, {
          ...leaderboardScore,
          sessionStartTime,
          sessionEndTime
        }),
        this.saveSkillScore(profileId, sessionId, {
          ...skillScore,
          sessionStartTime,
          sessionEndTime
        })
      ]);

      // Check if any failed
      const errors = [];
      if (!normalResult.success) errors.push(`Normal Score: ${normalResult.error}`);
      if (!leaderboardResult.success) errors.push(`Leaderboard Score: ${leaderboardResult.error}`);
      if (!skillResult.success) errors.push(`Skill Score: ${skillResult.error}`);

      if (errors.length > 0) {
        throw new Error(`Failed to save some scores: ${errors.join(', ')}`);
      }

      return {
        success: true,
        results: {
          normalScore: normalResult,
          leaderboardScore: leaderboardResult,
          skillScore: skillResult
        }
      };
    } catch (error) {
      console.error('Error saving all scores:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Get user's current totals
  async getUserCurrentTotals(profileId) {
    try {
      const { data: profile, error } = await supabase
        .from('profiles')
        .select('current_leaderboard_points, current_skill_points, highest_score')
        .eq('id', profileId)
        .single();

      if (error) {
        throw new Error(error.message);
      }

      return {
        success: true,
        data: {
          leaderboardPoints: profile.current_leaderboard_points || 0,
          skillPoints: profile.current_skill_points || 0,
          highestScore: profile.highest_score || 0
        }
      };
    } catch (error) {
      console.error('Error getting user current totals:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Get leaderboards from separate tables
  async getLeaderboards(type = 'normal', limit = 50) {
    try {
      let query;
      let tableName;
      let orderColumn;

      switch (type) {
        case 'leaderboard':
          tableName = 'leaderboard_scores';
          orderColumn = 'total_accumulated_points';
          break;
        case 'skill':
          tableName = 'skill_scores';
          orderColumn = 'total_accumulated_points';
          break;
        case 'normal':
        default:
          tableName = 'normal_scores';
          orderColumn = 'score';
          break;
      }

      const { data, error } = await supabase
        .from(tableName)
        .select(`
          *,
          profiles!inner(username, level, avatar)
        `)
        .order(orderColumn, { ascending: false })
        .limit(limit);

      if (error) {
        throw new Error(error.message);
      }

      return {
        success: true,
        data: data || []
      };
    } catch (error) {
      console.error('Error getting leaderboards:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }
}

module.exports = new ScoreService(); 