const express = require('express');
const jwt = require('jsonwebtoken');
const { body, validationResult } = require('express-validator');
const userService = require('../services/userService');
const { authMiddleware } = require('../middleware/auth');

const router = express.Router();

// Generate JWT token
const generateToken = (userId) => {
  return jwt.sign({ id: userId }, process.env.JWT_SECRET, {
    expiresIn: process.env.JWT_EXPIRES_IN || '7d'
  });
};

// ========== AUTHENTICATION METHOD CHOICE ==========

// @route   POST /api/auth/choose-method
// @desc    First-time user chooses authentication method and creates account
// @access  Public
router.post('/choose-method', [
  body('authMethod')
    .isIn(['steam', 'email', 'linked'])
    .withMessage('Authentication method must be steam, email, or linked'),
  body('username')
    .isLength({ min: 3, max: 20 })
    .withMessage('Username must be between 3 and 20 characters')
    .matches(/^[a-zA-Z0-9_]+$/)
    .withMessage('Username can only contain letters, numbers, and underscores'),
  // Conditional validation based on auth method
  body('email')
    .if(body('authMethod').isIn(['email', 'linked']))
    .isEmail()
    .withMessage('Valid email is required for email or linked authentication'),
  body('password')
    .if(body('authMethod').isIn(['email', 'linked']))
    .isLength({ min: 6 })
    .withMessage('Password must be at least 6 characters for email or linked authentication'),
  body('steamId')
    .if(body('authMethod').isIn(['steam', 'linked']))
    .notEmpty()
    .withMessage('Steam ID is required for Steam or linked authentication')
], async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { authMethod, username, email, password, steamId, steamData } = req.body;

    let user;
    switch (authMethod) {
      case 'steam':
        user = await userService.createSteamUser({
          steamId,
          username: username || steamData?.username,
          avatar: steamData?.avatar
        });
        break;
        
      case 'email':
        user = await userService.createUser({
          username,
          email,
          password
        });
        break;
        
      case 'linked':
        user = await userService.createLinkedUser({
          username,
          email,
          password,
          steamId,
          avatar: steamData?.avatar
        });
        break;
    }

    // Generate JWT token
    const token = jwt.sign(
      { 
        id: user.id, 
        username: user.username,
        authMethod: user.auth_method 
      },
      process.env.JWT_SECRET,
      { expiresIn: process.env.JWT_EXPIRES_IN || '7d' }
    );

    res.status(201).json({
      success: true,
      data: {
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          steamId: user.steam_id,
          avatar: user.avatar,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins
        },
        token
      }
    });
  } catch (error) {
    next(error);
  }
});

// ========== STEAM AUTHENTICATION ==========

// @route   POST /api/auth/steam
// @desc    Authenticate with Steam (for existing Steam users)
// @access  Public
router.post('/steam', [
  body('steamId')
    .notEmpty()
    .withMessage('Steam ID is required'),
  body('steamData.username')
    .optional()
    .isLength({ min: 1, max: 50 })
    .withMessage('Steam username must be 1-50 characters'),
], async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { steamId, steamData } = req.body;

    const user = await userService.loginSteamUser(steamId, steamData);

    if (!user) {
      return res.status(404).json({
        success: false,
        error: 'Steam account not found. Please register first.'
      });
    }

    // Generate JWT token
    const token = jwt.sign(
      { 
        id: user.id, 
        username: user.username,
        authMethod: user.auth_method 
      },
      process.env.JWT_SECRET,
      { expiresIn: process.env.JWT_EXPIRES_IN || '7d' }
    );

    res.json({
      success: true,
      data: {
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          steamId: user.steam_id,
          avatar: user.avatar,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins
        },
        token
      }
    });
  } catch (error) {
    next(error);
  }
});

// ========== EMAIL AUTHENTICATION (EXISTING, UPDATED) ==========

// @route   POST /api/auth/register
// @desc    Register user with email and password
// @access  Public
router.post('/register', [
  body('username')
    .isLength({ min: 3, max: 20 })
    .withMessage('Username must be between 3 and 20 characters')
    .matches(/^[a-zA-Z0-9_]+$/)
    .withMessage('Username can only contain letters, numbers, and underscores'),
  body('email')
    .isEmail()
    .withMessage('Please enter a valid email'),
  body('password')
    .isLength({ min: 6 })
    .withMessage('Password must be at least 6 characters')
], async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { username, email, password } = req.body;

    const user = await userService.createUser({
      username,
      email,
      password
    });

    // Generate JWT token
    const token = jwt.sign(
      { 
        id: user.id, 
        username: user.username,
        authMethod: user.auth_method 
      },
      process.env.JWT_SECRET,
      { expiresIn: process.env.JWT_EXPIRES_IN || '7d' }
    );

    res.status(201).json({
      success: true,
      data: {
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins
        },
        token
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   POST /api/auth/login
// @desc    Login user with email and password
// @access  Public
router.post('/login', [
  body('email')
    .isEmail()
    .withMessage('Please enter a valid email'),
  body('password')
    .notEmpty()
    .withMessage('Password is required')
], async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { email, password } = req.body;

    const user = await userService.authenticateEmailUser(email, password);

    // Generate JWT token
    const token = jwt.sign(
      { 
        id: user.id, 
        username: user.username,
        authMethod: user.auth_method 
      },
      process.env.JWT_SECRET,
      { expiresIn: process.env.JWT_EXPIRES_IN || '7d' }
    );

    res.json({
      success: true,
      data: {
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          steamId: user.steam_id,
          avatar: user.avatar,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins
        },
        token
      }
    });
  } catch (error) {
    next(error);
  }
});

// ========== ACCOUNT LINKING ==========

// @route   POST /api/auth/link-steam
// @desc    Link Steam account to existing email account
// @access  Private
router.post('/link-steam', [
  body('steamId')
    .notEmpty()
    .withMessage('Steam ID is required'),
], authMiddleware, async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { steamId } = req.body;
    const userEmail = req.user.email;

    if (!userEmail) {
      return res.status(400).json({
        success: false,
        error: 'Cannot link Steam to Steam-only account'
      });
    }

    const user = await userService.linkSteamToEmailAccount(userEmail, steamId);

    res.json({
      success: true,
      data: {
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          steamId: user.steam_id,
          avatar: user.avatar,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins
        }
      }
    });
  } catch (error) {
    next(error);
  }
});

// ========== USER PROFILE (UPDATED) ==========

// @route   GET /api/auth/me
// @desc    Get current user profile
// @access  Private
router.get('/me', authMiddleware, async (req, res, next) => {
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
          id: user.id,
          username: user.username,
          email: user.email,
          steamId: user.steam_id,
          avatar: user.avatar,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins,
          lastLogin: user.last_login,
          createdAt: user.created_at
        }
      }
    });
  } catch (error) {
    next(error);
  }
});

// Check if email exists
router.get('/check-email', async (req, res) => {
    try {
        const { email } = req.query;
        
        if (!email) {
            return res.status(400).json({
                success: false,
                error: 'Email is required'
            });
        }
        
        // Check if email exists in database
        const exists = await userService.checkEmailExists(email);
        
        res.json({
            success: true,
            exists: exists
        });
        
    } catch (error) {
        console.error('Email check error:', error);
        res.status(500).json({
            success: false,
            error: 'Server error checking email'
        });
    }
});

// ========== DEVELOPMENT AUTHENTICATION ==========

// @route   POST /api/auth/dev-login
// @desc    Development login (creates test user with valid JWT)
// @access  Public (development only)
router.post('/dev-login', async (req, res, next) => {
  try {
    // Only allow in development mode
    if (process.env.NODE_ENV === 'production') {
      return res.status(403).json({
        success: false,
        error: 'Development authentication not available in production'
      });
    }

    // Create or find test user
    let user;
    try {
      user = await userService.findUserByEmail('test@development.local');
    } catch (error) {
      // User doesn't exist, create it
      user = await userService.createUser({
        username: 'TestUser',
        email: 'test@development.local',
        password: 'testpassword123'
      });
    }

    // Generate JWT token
    const token = jwt.sign(
      { 
        id: user.id, 
        username: user.username,
        authMethod: user.auth_method || 'email'
      },
      process.env.JWT_SECRET,
      { expiresIn: process.env.JWT_EXPIRES_IN || '7d' }
    );

    res.json({
      success: true,
      data: {
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          authMethod: user.auth_method,
          level: user.level,
          experience: user.experience,
          skillPoints: user.skill_points,
          coins: user.coins
        },
        token
      }
    });
  } catch (error) {
    next(error);
  }
});

module.exports = router; 