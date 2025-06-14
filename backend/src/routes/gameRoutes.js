const express = require('express');
const router = express.Router();
const GameSessionService = require('../services/GameSessionService');
const ScoreService = require('../services/ScoreService');
const supabase = require('../config/supabase');
const { authMiddleware } = require('../middleware/auth');

// All routes require authentication
router.use(authMiddleware);

// Start a new game session
router.post('/session/start', async (req, res) => {
  try {
    const { gameMode = 'normal' } = req.body;
    const userId = req.user.id;

    console.log(`Starting session for user ${userId}, mode: ${gameMode}`);

    const result = await GameSessionService.startSession(userId, gameMode);

    if (!result.success) {
      return res.status(400).json({
        error: result.error
      });
    }

    res.json({
      success: true,
      sessionId: result.sessionId,
      profileId: result.profileId,
      message: 'Game session started successfully'
    });
  } catch (error) {
    console.error('Error starting game session:', error);
    res.status(500).json({
      error: 'Failed to start game session'
    });
  }
});

// Save game progress during session
router.post('/progress/save', async (req, res) => {
  try {
    const {
      sessionId,
      currentScore,
      leaderboardPoints,
      skillPoints,
      survivalTime,
      kills,
      enemiesSpawned,
      damageDealt,
      damageTaken,
      waveReached
    } = req.body;

    if (!sessionId) {
      return res.status(400).json({
        error: 'Session ID is required'
      });
    }

    const result = await GameSessionService.saveProgress(sessionId, {
      currentScore: currentScore || 0,
      leaderboardPoints: leaderboardPoints || 0,
      skillPoints: skillPoints || 0,
      survivalTime: survivalTime || 0,
      kills: kills || 0,
      enemiesSpawned: enemiesSpawned || 0,
      damageDealt: damageDealt || 0,
      damageTaken: damageTaken || 0,
      waveReached: waveReached || 1
    });

    if (!result.success) {
      return res.status(400).json({
        error: result.error
      });
    }

    res.json({
      success: true,
      message: 'Progress saved successfully'
    });
  } catch (error) {
    console.error('Error saving progress:', error);
    res.status(500).json({
      error: 'Failed to save progress'
    });
  }
});

// End game session and save all scores
router.post('/session/end', async (req, res) => {
  try {
    const {
      sessionId,
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
    } = req.body;

    if (!sessionId) {
      return res.status(400).json({
        error: 'Session ID is required'
      });
    }

    console.log(`Ending session ${sessionId} for user ${req.user.id}`);

    const result = await GameSessionService.endSession(sessionId, {
      finalScore: finalScore || 0,
      leaderboardPointsEarned: leaderboardPointsEarned || 0,
      skillPointsEarned: skillPointsEarned || 0,
      survivalTime: survivalTime || 0,
      kills: kills || 0,
      enemiesSpawned: enemiesSpawned || 0,
      damageDealt: damageDealt || 0,
      damageTaken: damageTaken || 0,
      waveReached: waveReached || 1,
      endReason: endReason || 'player_death'
    });

    if (!result.success) {
      return res.status(400).json({
        error: result.error
      });
    }

    res.json({
      success: true,
      sessionId: result.sessionId,
      profileId: result.profileId,
      scoresResult: result.scoresResult,
      message: 'Game session ended and scores saved successfully'
    });
  } catch (error) {
    console.error('Error ending game session:', error);
    res.status(500).json({
      error: 'Failed to end game session'
    });
  }
});

// Get user's current stats (for loading into game)
router.get('/stats', async (req, res) => {
  try {
    const userId = req.user.id;
    const result = await GameSessionService.getUserStats(userId);

    if (!result.success) {
      return res.status(400).json({
        error: result.error
      });
    }

    res.json({
      success: true,
      stats: result.stats
    });
  } catch (error) {
    console.error('Error getting user stats:', error);
    res.status(500).json({
      error: 'Failed to get user stats'
    });
  }
});

// Get active session for user
router.get('/session/active', async (req, res) => {
  try {
    const userId = req.user.id;
    const result = await GameSessionService.getActiveSession(userId);

    if (!result.success) {
      return res.status(400).json({
        error: result.error
      });
    }

    res.json({
      success: true,
      activeSession: result.session
    });
  } catch (error) {
    console.error('Error getting active session:', error);
    res.status(500).json({
      error: 'Failed to get active session'
    });
  }
});

// Get leaderboards
router.get('/leaderboards/:type?', async (req, res) => {
  try {
    const { type = 'normal' } = req.params;
    const { limit = 50 } = req.query;

    // Validate type parameter
    const validTypes = ['normal', 'leaderboard', 'skill'];
    if (!validTypes.includes(type)) {
      return res.status(400).json({
        error: `Invalid leaderboard type. Must be one of: ${validTypes.join(', ')}`
      });
    }

    const result = await GameSessionService.getLeaderboards(type, parseInt(limit));

    if (!result.success) {
      return res.status(400).json({
        error: result.error
      });
    }

    res.json({
      success: true,
      type: type,
      leaderboard: result.data
    });
  } catch (error) {
    console.error('Error getting leaderboards:', error);
    res.status(500).json({
      error: 'Failed to get leaderboards'
    });
  }
});

// Save individual score types (additional endpoints if needed)
router.post('/scores/normal', async (req, res) => {
  try {
    const userId = req.user.id;
    const { sessionId, scoreData } = req.body;

    // Get user's profile
    const { data: profile } = await supabase
      .from('profiles')
      .select('id')
      .eq('user_id', userId)
      .single();

    if (!profile) {
      return res.status(404).json({ error: 'User profile not found' });
    }

    const result = await ScoreService.saveNormalScore(profile.id, sessionId, scoreData);

    if (!result.success) {
      return res.status(400).json({ error: result.error });
    }

    res.json({
      success: true,
      normalScore: result.normalScore,
      isPersonalBest: result.isPersonalBest
    });
  } catch (error) {
    console.error('Error saving normal score:', error);
    res.status(500).json({ error: 'Failed to save normal score' });
  }
});

router.post('/scores/leaderboard', async (req, res) => {
  try {
    const userId = req.user.id;
    const { sessionId, leaderboardData } = req.body;

    // Get user's profile
    const { data: profile } = await supabase
      .from('profiles')
      .select('id')
      .eq('user_id', userId)
      .single();

    if (!profile) {
      return res.status(404).json({ error: 'User profile not found' });
    }

    const result = await ScoreService.saveLeaderboardScore(profile.id, sessionId, leaderboardData);

    if (!result.success) {
      return res.status(400).json({ error: result.error });
    }

    res.json({
      success: true,
      leaderboardScore: result.leaderboardScore,
      newTotal: result.newTotal
    });
  } catch (error) {
    console.error('Error saving leaderboard score:', error);
    res.status(500).json({ error: 'Failed to save leaderboard score' });
  }
});

router.post('/scores/skill', async (req, res) => {
  try {
    const userId = req.user.id;
    const { sessionId, skillData } = req.body;

    // Get user's profile
    const { data: profile } = await supabase
      .from('profiles')
      .select('id')
      .eq('user_id', userId)
      .single();

    if (!profile) {
      return res.status(404).json({ error: 'User profile not found' });
    }

    const result = await ScoreService.saveSkillScore(profile.id, sessionId, skillData);

    if (!result.success) {
      return res.status(400).json({ error: result.error });
    }

    res.json({
      success: true,
      skillScore: result.skillScore,
      newTotal: result.newTotal
    });
  } catch (error) {
    console.error('Error saving skill score:', error);
    res.status(500).json({ error: 'Failed to save skill score' });
  }
});

module.exports = router; 