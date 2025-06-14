const express = require('express');
const { body, validationResult } = require('express-validator');
const userService = require('../services/userService');
const { authMiddleware } = require('../middleware/auth');

const router = express.Router();

// @route   GET /api/users/profile
// @desc    Get user profile
// @access  Private
router.get('/profile', authMiddleware, async (req, res, next) => {
  try {
    const user = await userService.findUserById(req.user.id);
    
    if (!user) {
      return res.status(404).json({
        success: false,
        error: 'User not found'
      });
    }

    res.json({
      success: true,
      data: {
        user: {
          id: user.user_id,
          username: user.username,
          email: user.email,
          avatar: user.avatar,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins,
          gameStats: {
            totalPlayTime: user.total_play_time,
            gamesPlayed: user.games_played,
            bestScore: user.best_score,
            longestSurvivalTime: user.longest_survival_time,
            totalKills: user.total_kills,
            totalDeaths: user.total_deaths,
            totalDamageDealt: user.total_damage_dealt,
            totalDamageTaken: user.total_damage_taken
          },
          preferences: {
            soundEnabled: user.sound_enabled,
            musicEnabled: user.music_enabled,
            volume: user.volume,
            graphicsQuality: user.graphics_quality
          },
          createdAt: user.created_at,
          lastLogin: user.last_login
        }
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   PUT /api/users/profile
// @desc    Update user profile
// @access  Private
router.put('/profile', [
  body('username')
    .optional()
    .trim()
    .isLength({ min: 3, max: 20 })
    .withMessage('Username must be between 3 and 20 characters')
    .matches(/^[a-zA-Z0-9_]+$/)
    .withMessage('Username can only contain letters, numbers, and underscores'),
  body('email')
    .optional()
    .isEmail()
    .normalizeEmail()
    .withMessage('Please enter a valid email')
], authMiddleware, async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { username, email } = req.body;
    const currentUser = await userService.findUserById(req.user.id);
    
    if (!currentUser) {
      return res.status(404).json({
        success: false,
        error: 'User not found'
      });
    }

    const updateData = {};

    if (username && username !== currentUser.username) {
      // Check if username is already taken
      const existingUser = await userService.findUserByUsername(username);
      if (existingUser && existingUser.user_id !== currentUser.user_id) {
        return res.status(400).json({
          success: false,
          error: 'Username already taken'
        });
      }
      updateData.username = username;
    }

    if (email && email !== currentUser.email) {
      // Check if email is already taken
      const existingUser = await userService.findUserByEmail(email);
      if (existingUser && existingUser.user_id !== currentUser.user_id) {
        return res.status(400).json({
          success: false,
          error: 'Email already registered'
        });
      }
      updateData.email = email;
    }

    const updatedUser = await userService.updateUser(currentUser.user_id, updateData);

    res.json({
      success: true,
      data: { user: updatedUser }
    });
  } catch (error) {
    next(error);
  }
});

// @route   PUT /api/users/preferences
// @desc    Update user preferences
// @access  Private
router.put('/preferences', [
  body('soundEnabled')
    .optional()
    .isBoolean()
    .withMessage('Sound enabled must be a boolean'),
  body('musicEnabled')
    .optional()
    .isBoolean()
    .withMessage('Music enabled must be a boolean'),
  body('volume')
    .optional()
    .isInt({ min: 0, max: 100 })
    .withMessage('Volume must be between 0 and 100'),
  body('graphicsQuality')
    .optional()
    .isIn(['low', 'medium', 'high', 'ultra'])
    .withMessage('Graphics quality must be low, medium, high, or ultra')
], authMiddleware, async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { soundEnabled, musicEnabled, volume, graphicsQuality } = req.body;
    
    const updateData = {};
    if (soundEnabled !== undefined) updateData.sound_enabled = soundEnabled;
    if (musicEnabled !== undefined) updateData.music_enabled = musicEnabled;
    if (volume !== undefined) updateData.volume = volume;
    if (graphicsQuality !== undefined) updateData.graphics_quality = graphicsQuality;

    const updatedUser = await userService.updateUser(req.user.id, updateData);

    res.json({
      success: true,
      data: {
        preferences: {
          soundEnabled: updatedUser.sound_enabled,
          musicEnabled: updatedUser.music_enabled,
          volume: updatedUser.volume,
          graphicsQuality: updatedUser.graphics_quality
        }
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/users/stats
// @desc    Get user game statistics
// @access  Private
router.get('/stats', authMiddleware, async (req, res, next) => {
  try {
    const stats = await userService.getUserStats(req.user.id);
    
    res.json({
      success: true,
      data: stats
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/users/skills
// @desc    Get user skills
// @access  Private
router.get('/skills', authMiddleware, async (req, res, next) => {
  try {
    const skills = await userService.getUserSkills(req.user.id);
    
    res.json({
      success: true,
      data: skills
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/users/inventory
// @desc    Get user inventory
// @access  Private
router.get('/inventory', authMiddleware, async (req, res, next) => {
  try {
    const inventory = await userService.getUserInventory(req.user.id);
    
    res.json({
      success: true,
      data: inventory
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/users/:username
// @desc    Get public user profile by username
// @access  Public
router.get('/:username', async (req, res, next) => {
  try {
    const { username } = req.params;

    const user = await userService.findUserByUsername(username);

    if (!user) {
      return res.status(404).json({
        success: false,
        error: 'User not found'
      });
    }

    res.json({
      success: true,
      data: {
        user: {
          username: user.username,
          profile: {
            level: user.level,
            avatar: user.avatar,
            bestScore: user.best_score,
            gamesPlayed: user.games_played,
            longestSurvivalTime: user.longest_survival_time
          },
          gameStats: {
            totalKills: user.total_kills,
            totalDeaths: user.total_deaths
          },
          memberSince: user.created_at
        }
      }
    });
  } catch (error) {
    next(error);
  }
});

module.exports = router; 