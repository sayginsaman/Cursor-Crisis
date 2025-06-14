const supabase = require('../config/supabase');
const ScoreService = require('./ScoreService');

class GameSessionService {
  constructor() {}

  // Start a new game session
  async startSession(userId, gameMode = 'normal') {
    try {
      // userId is actually the profile ID from the JWT token
      const profileId = userId;

      // Verify the profile exists
      const { data: profile, error: profileError } = await supabase
        .from('profiles')
        .select('id')
        .eq('id', profileId)
        .single();

      if (profileError || !profile) {
        throw new Error('User profile not found');
      }

      // Create new game session
      const { data: session, error: sessionError } = await supabase
        .from('game_sessions')
        .insert([{
          profile_id: profile.id,
          game_mode: gameMode,
          status: 'in_progress',
          survival_time: 0,
          score: 0,
          leaderboard_points_earned: 0,
          skill_points_earned: 0,
          started_at: new Date().toISOString()
        }])
        .select()
        .single();

      if (sessionError) {
        throw new Error(`Failed to create session: ${sessionError.message}`);
      }

      console.log(`Started session ${session.id} for profile ${profile.id}`);

      return {
        success: true,
        sessionId: session.id,
        profileId: profile.id
      };
    } catch (error) {
      console.error('Error starting session:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Save game progress during session
  async saveProgress(sessionId, progressData) {
    try {
      const {
        currentScore,
        leaderboardPoints,
        skillPoints,
        survivalTime,
        kills,
        enemiesSpawned,
        damageDealt,
        damageTaken,
        waveReached
      } = progressData;

      // Update game session with current progress
      const { data: updatedSession, error: updateError } = await supabase
        .from('game_sessions')
        .update({
          score: currentScore,
          leaderboard_points_earned: leaderboardPoints,
          skill_points_earned: skillPoints,
          survival_time: survivalTime,
          kills: kills || 0,
          enemies_spawned: enemiesSpawned || 0,
          damage_dealt: damageDealt || 0,
          damage_taken: damageTaken || 0,
          wave_reached: waveReached || 1,
          updated_at: new Date().toISOString()
        })
        .eq('id', sessionId)
        .select()
        .single();

      if (updateError) {
        throw new Error(`Failed to save progress: ${updateError.message}`);
      }

      // Also save snapshot for real-time tracking
      await supabase
        .from('game_progress_snapshots')
        .insert([{
          session_id: sessionId,
          profile_id: updatedSession.profile_id,
          score: currentScore,
          leaderboard_points: leaderboardPoints,
          skill_points: skillPoints,
          survival_time: survivalTime,
          kills: kills || 0,
          enemies_spawned: enemiesSpawned || 0,
          damage_dealt: damageDealt || 0,
          damage_taken: damageTaken || 0,
          wave_reached: waveReached || 1
        }]);

      return {
        success: true,
        session: updatedSession
      };
    } catch (error) {
      console.error('Error saving progress:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // End game session and save all scores to separate tables
  async endSession(sessionId, finalData) {
    try {
      const {
        finalScore,
        leaderboardPointsEarned,
        skillPointsEarned,
        survivalTime,
        kills,
        enemiesSpawned,
        damageDealt,
        damageTaken,
        waveReached,
        endReason
      } = finalData;

      console.log(`Ending session ${sessionId} with final data:`, finalData);

      // Get session info
      const { data: session, error: sessionError } = await supabase
        .from('game_sessions')
        .select('*')
        .eq('id', sessionId)
        .single();

      if (sessionError || !session) {
        throw new Error('Session not found');
      }

      const sessionEndTime = new Date().toISOString();

      // Update session as completed
      const { error: updateError } = await supabase
        .from('game_sessions')
        .update({
          status: 'completed',
          score: finalScore,
          leaderboard_points_earned: leaderboardPointsEarned,
          skill_points_earned: skillPointsEarned,
          survival_time: survivalTime,
          kills: kills || 0,
          enemies_spawned: enemiesSpawned || 0,
          damage_dealt: damageDealt || 0,
          damage_taken: damageTaken || 0,
          wave_reached: waveReached || 1,
          end_reason: endReason || 'player_death',
          ended_at: sessionEndTime
        })
        .eq('id', sessionId);

      if (updateError) {
        throw new Error(`Failed to update session: ${updateError.message}`);
      }

      // Save all scores to separate tables using ScoreService
      const scoreResult = await ScoreService.saveAllScores(session.profile_id, sessionId, {
        normalScore: {
          score: finalScore,
          survivalTime: survivalTime,
          kills: kills || 0,
          enemiesSpawned: enemiesSpawned || 0,
          damageDealt: damageDealt || 0,
          damageTaken: damageTaken || 0,
          waveReached: waveReached || 1
        },
        leaderboardScore: {
          pointsEarnedThisSession: leaderboardPointsEarned,
          survivalTime: survivalTime
        },
        skillScore: {
          pointsEarnedThisSession: skillPointsEarned,
          survivalTime: survivalTime
        },
        sessionStartTime: session.started_at,
        sessionEndTime: sessionEndTime
      });

      if (!scoreResult.success) {
        console.error('Failed to save scores:', scoreResult.error);
        // Continue anyway, session is already marked as completed
      }

      console.log(`Session ${sessionId} ended successfully. Scores saved:`, scoreResult.success);

      return {
        success: true,
        sessionId: sessionId,
        profileId: session.profile_id,
        scoresResult: scoreResult
      };
    } catch (error) {
      console.error('Error ending session:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Get user's current stats from their profile
  async getUserStats(userId) {
    try {
      // userId is actually the profile ID from the JWT token
      const profileId = userId;

      const { data: profile, error: profileError } = await supabase
        .from('profiles')
        .select('id, level, skill_points, coins')
        .eq('id', profileId)
        .single();

      if (profileError || !profile) {
        throw new Error('User profile not found');
      }

      return {
        success: true,
        stats: {
          profileId: profile.id,
          level: profile.level || 1,
          skillPoints: profile.skill_points || 0,
          coins: profile.coins || 50
        }
      };
    } catch (error) {
      console.error('Error getting user stats:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Get active session for user
  async getActiveSession(userId) {
    try {
      // userId is actually the profile ID from the JWT token
      const profileId = userId;

      const { data: profile, error: profileError } = await supabase
        .from('profiles')
        .select('id')
        .eq('id', profileId)
        .single();

      if (profileError || !profile) {
        throw new Error('User profile not found');
      }

      const { data: session, error: sessionError } = await supabase
        .from('game_sessions')
        .select('*')
        .eq('profile_id', profile.id)
        .eq('status', 'in_progress')
        .order('started_at', { ascending: false })
        .limit(1)
        .single();

      if (sessionError && sessionError.code !== 'PGRST116') {
        throw new Error(sessionError.message);
      }

      return {
        success: true,
        session: session || null
      };
    } catch (error) {
      console.error('Error getting active session:', error);
      return {
        success: false,
        error: error.message
      };
    }
  }

  // Get leaderboards using ScoreService
  async getLeaderboards(type = 'normal', limit = 50) {
    return await ScoreService.getLeaderboards(type, limit);
  }
}

module.exports = new GameSessionService(); 