const express = require('express');
const { body, validationResult } = require('express-validator');
const User = require('../models/User');
const DLCItem = require('../models/DLCItem');
const { authMiddleware } = require('../middleware/auth');

const router = express.Router();

// @route   GET /api/dlc/items
// @desc    Get all available DLC items
// @access  Public
router.get('/items', async (req, res, next) => {
  try {
    const { category, rarity, featured } = req.query;
    
    let filter = { 'availability.isAvailable': true };
    
    if (category) filter.category = category;
    if (rarity) filter.rarity = rarity;
    if (featured === 'true') filter.featured = true;

    const items = await DLCItem.find(filter)
      .sort({ featured: -1, rarity: 1, price: 1 });

    // Group items by category
    const groupedItems = items.reduce((acc, item) => {
      if (!acc[item.category]) {
        acc[item.category] = [];
      }
      acc[item.category].push(item);
      return acc;
    }, {});

    res.json({
      success: true,
      data: {
        items: groupedItems,
        totalItems: items.length,
        categories: Object.keys(groupedItems)
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/dlc/featured
// @desc    Get featured DLC items
// @access  Public
router.get('/featured', async (req, res, next) => {
  try {
    const featuredItems = await DLCItem.find({ 
      'availability.isAvailable': true, 
      featured: true 
    })
    .sort({ featuredOrder: 1 })
    .limit(10);

    res.json({
      success: true,
      data: { items: featuredItems }
    });
  } catch (error) {
    next(error);
  }
});

// @route   POST /api/dlc/purchase
// @desc    Purchase a DLC item
// @access  Private
router.post('/purchase', [
  body('itemId')
    .notEmpty()
    .withMessage('Item ID is required'),
  body('paymentMethod')
    .isIn(['steam', 'paypal', 'stripe'])
    .withMessage('Invalid payment method')
], authMiddleware, async (req, res, next) => {
  try {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({
        success: false,
        errors: errors.array()
      });
    }

    const { itemId, paymentMethod } = req.body;

    // Find the DLC item
    const item = await DLCItem.findById(itemId);
    if (!item || !item.availability.isAvailable) {
      return res.status(404).json({
        success: false,
        error: 'Item not found or unavailable'
      });
    }

    const user = await User.findById(req.user._id);

    // Check if user already owns the item
    let alreadyOwns = false;
    if (item.category === 'cursor') {
      alreadyOwns = user.inventory.cursors.some(cursor => cursor.cursorId === item.itemId);
    } else if (item.category === 'theme') {
      alreadyOwns = user.inventory.themes.some(theme => theme.themeId === item.itemId);
    }

    if (alreadyOwns) {
      return res.status(400).json({
        success: false,
        error: 'You already own this item'
      });
    }

    // In a real application, you would integrate with payment processors here
    // For now, we'll simulate a successful purchase
    
    // Add item to user's inventory
    if (item.category === 'cursor') {
      user.inventory.cursors.push({
        cursorId: item.itemId,
        name: item.name,
        rarity: item.rarity,
        purchasedAt: new Date()
      });
    } else if (item.category === 'theme') {
      user.inventory.themes.push({
        themeId: item.itemId,
        name: item.name,
        purchasedAt: new Date()
      });
    }

    await user.save();

    res.json({
      success: true,
      data: {
        message: 'Purchase successful!',
        item: {
          id: item._id,
          name: item.name,
          category: item.category,
          rarity: item.rarity
        },
        inventory: user.inventory
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/dlc/owned
// @desc    Get user's owned DLC items
// @access  Private
router.get('/owned', authMiddleware, async (req, res, next) => {
  try {
    const user = await User.findById(req.user._id);

    // Get full details of owned items
    const ownedCursorIds = user.inventory.cursors.map(c => c.cursorId);
    const ownedThemeIds = user.inventory.themes.map(t => t.themeId);

    const ownedCursors = await DLCItem.find({
      itemId: { $in: ownedCursorIds },
      category: 'cursor'
    });

    const ownedThemes = await DLCItem.find({
      itemId: { $in: ownedThemeIds },
      category: 'theme'
    });

    res.json({
      success: true,
      data: {
        cursors: ownedCursors,
        themes: ownedThemes,
        activeCursor: user.inventory.activeCursor,
        activeTheme: user.inventory.activeTheme,
        totalItems: ownedCursors.length + ownedThemes.length
      }
    });
  } catch (error) {
    next(error);
  }
});

// @route   GET /api/dlc/categories
// @desc    Get available DLC categories with counts
// @access  Public
router.get('/categories', async (req, res, next) => {
  try {
    const categories = await DLCItem.aggregate([
      { $match: { 'availability.isAvailable': true } },
      {
        $group: {
          _id: '$category',
          count: { $sum: 1 },
          minPrice: { $min: '$price' },
          maxPrice: { $max: '$price' },
          rarities: { $addToSet: '$rarity' }
        }
      },
      { $sort: { _id: 1 } }
    ]);

    res.json({
      success: true,
      data: { categories }
    });
  } catch (error) {
    next(error);
  }
});

module.exports = router; 