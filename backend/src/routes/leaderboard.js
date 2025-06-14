const express = require('express');
const GameSessionService = require('../services/GameSessionService');
const { optionalAuth } = require('../middleware/auth');

const router = express.Router();
const gameSessionService = GameSessionService;

// @route   GET /api/leaderboard/scores
// @desc    Get top game scores leaderboard (individual session scores)
// @access  Public (with optional auth for user ranking)
router.get('/scores', optionalAuth, async (req, res, next) => {
  try {
    const limit = Math.min(parseInt(req.query.limit) || 50, 100);
    const timeframe = req.query.timeframe || 'all'; // all, daily, weekly, monthly

    const leaderboard = await gameSessionService.getLeaderboard('score', limit, timeframe);

    res.json({
      success: true,
      data: {
        leaderboard,
        type: 'Game Scores',
        description: 'Highest individual game session scores',
        timeframe,
        totalEntries: leaderboard.length
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/leaderboard/leaderboard-points
// @desc    Get leaderboard points ranking (accumulated points)
// @access  Public
router.get('/leaderboard-points', optionalAuth, async (req, res, next) => {
  try {
    const limit = Math.min(parseInt(req.query.limit) || 50, 100);

    const leaderboard = await gameSessionService.getLeaderboard('leaderboard_points', limit, 'all');

    res.json({
      success: true,
      data: {
        leaderboard,
        type: 'Leaderboard Points',
        description: 'Players ranked by accumulated leaderboard points',
        timeframe: 'all',
        totalEntries: leaderboard.length
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/leaderboard/survival
// @desc    Get longest survival times leaderboard
// @access  Public
router.get('/survival', optionalAuth, async (req, res, next) => {
  try {
    const limit = Math.min(parseInt(req.query.limit) || 50, 100);
    const timeframe = req.query.timeframe || 'all';

    const leaderboard = await gameSessionService.getLeaderboard('survival', limit, timeframe);

    res.json({
      success: true,
      data: {
        leaderboard,
        type: 'Survival Time',
        description: 'Longest survival times in seconds',
        timeframe,
        totalEntries: leaderboard.length
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/leaderboard/all
// @desc    Get all leaderboard types for dashboard display
// @access  Public
router.get('/all', async (req, res, next) => {
  try {
    const limit = Math.min(parseInt(req.query.limit) || 10, 50);

    // Get top entries from each leaderboard type
    const [gameScores, leaderboardPoints, survivalTimes] = await Promise.all([
      gameSessionService.getLeaderboard('score', limit, 'all'),
      gameSessionService.getLeaderboard('leaderboard_points', limit, 'all'),
      gameSessionService.getLeaderboard('survival', limit, 'all')
    ]);

    res.json({
      success: true,
      data: {
        gameScores: {
          leaderboard: gameScores,
          type: 'Game Scores',
          description: 'Highest individual game session scores'
        },
        leaderboardPoints: {
          leaderboard: leaderboardPoints,
          type: 'Leaderboard Points',
          description: 'Players ranked by accumulated leaderboard points'
        },
        survivalTimes: {
          leaderboard: survivalTimes,
          type: 'Survival Time',
          description: 'Longest survival times'
        }
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/leaderboard/recent
// @desc    Get recent high scores (last 24 hours)
// @access  Public
router.get('/recent', async (req, res, next) => {
  try {
    const limit = Math.min(parseInt(req.query.limit) || 20, 50);

    const recentScores = await gameSessionService.getLeaderboard('score', limit, 'daily');

    res.json({
      success: true,
      data: {
        leaderboard: recentScores,
        type: 'Recent Scores',
        description: 'Recent high scores from the last 24 hours',
        timeframe: 'Last 24 hours',
        totalEntries: recentScores.length
      }
    });
  } catch (error) {
    next(error);
  }
});

module.exports = router; 