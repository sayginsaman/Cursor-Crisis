const supabase = require('../config/supabase');
const bcrypt = require('bcryptjs');

class UserService {
  async createUser(userData) {
    const { username, email, password } = userData;
    
    // Hash password
    const saltRounds = 10;
    const password_hash = await bcrypt.hash(password, saltRounds);
    
    const { data, error } = await supabase
      .from('profiles')
      .insert([{
        username,
        email,
        password_hash,
        auth_method: 'email'
      }])
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    // Remove password from response
    delete data.password_hash;
    return data;
  }

  async createSteamUser(steamData) {
    const { steamId, username, avatar } = steamData;
    
    // Check if Steam user already exists
    const existingUser = await this.findUserBySteamId(steamId);
    if (existingUser) {
      return existingUser;
    }
    
    const { data, error } = await supabase
      .from('profiles')
      .insert([{
        username: username || `Steam_${steamId.substring(0, 8)}`,
        steam_id: steamId,
        avatar: avatar || 'default-cursor.png',
        auth_method: 'steam'
      }])
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async findUserBySteamId(steamId) {
    const { data, error } = await supabase
      .from('profiles')
      .select('*')
      .eq('steam_id', steamId)
      .single();
    
    if (error && error.code !== 'PGRST116') { // PGRST116 = not found
      throw new Error(error.message);
    }
    
    return data;
  }

  async loginSteamUser(steamId, steamData = {}) {
    let user = await this.findUserBySteamId(steamId);
    
    if (!user) {
      // Create new Steam user if doesn't exist
      user = await this.createSteamUser({
        steamId,
        username: steamData.username,
        avatar: steamData.avatar
      });
    } else {
      // Update last login for existing user
      await this.updateLastLogin(user.id);
    }
    
    return user;
  }

  async createLinkedUser(userData) {
    const { username, email, password, steamId, avatar } = userData;
    
    // Hash password
    const saltRounds = 10;
    const password_hash = await bcrypt.hash(password, saltRounds);
    
    const { data, error } = await supabase
      .from('profiles')
      .insert([{
        username,
        email,
        password_hash,
        steam_id: steamId,
        avatar: avatar || 'default-cursor.png',
        auth_method: 'linked'
      }])
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    // Remove password from response
    delete data.password_hash;
    return data;
  }

  async linkSteamToEmailAccount(email, steamId) {
    const user = await this.findUserByEmail(email);
    if (!user) {
      throw new Error('Email account not found');
    }
    
    if (user.auth_method === 'steam') {
      throw new Error('Cannot link Steam to Steam-only account');
    }
    
    const { data, error } = await supabase
      .from('profiles')
      .update({
        steam_id: steamId,
        auth_method: 'linked'
      })
      .eq('id', user.id)
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    delete data.password_hash;
    return data;
  }

  async findUserByEmail(email) {
    const { data, error } = await supabase
      .from('profiles')
      .select('*')
      .eq('email', email)
      .single();
    
    if (error && error.code !== 'PGRST116') { // PGRST116 = not found
      throw new Error(error.message);
    }
    
    return data;
  }

  async findUserByUsername(username) {
    const { data, error } = await supabase
      .from('profiles')
      .select('*')
      .eq('username', username)
      .single();
    
    if (error && error.code !== 'PGRST116') {
      throw new Error(error.message);
    }
    
    return data;
  }

  async findUserById(id) {
    const { data, error } = await supabase
      .from('profiles')
      .select('*')
      .eq('id', id)
      .single();
    
    if (error && error.code !== 'PGRST116') {
      throw new Error(error.message);
    }
    
    if (data) {
      delete data.password_hash;
    }
    
    return data;
  }

  async updateUser(id, updateData) {
    const { data, error } = await supabase
      .from('profiles')
      .update(updateData)
      .eq('id', id)
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    delete data.password_hash;
    return data;
  }

  async updateLastLogin(id) {
    const { data, error } = await supabase
      .from('profiles')
      .update({ last_login: new Date().toISOString() })
      .eq('id', id)
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async verifyPassword(plainPassword, hashedPassword) {
    return await bcrypt.compare(plainPassword, hashedPassword);
  }

  async authenticateUser(credentials) {
    const { authMethod, email, password, steamId, steamData } = credentials;
    
    switch (authMethod) {
      case 'email':
        return await this.authenticateEmailUser(email, password);
      case 'steam':
        return await this.loginSteamUser(steamId, steamData);
      case 'linked':
        // For linked accounts, try both authentication methods
        if (email && password) {
          return await this.authenticateEmailUser(email, password);
        } else if (steamId) {
          return await this.loginSteamUser(steamId, steamData);
        } else {
          throw new Error('Invalid credentials for linked account');
        }
      default:
        throw new Error('Invalid authentication method');
    }
  }

  async authenticateEmailUser(email, password) {
    const { data: user, error } = await supabase
      .from('profiles')
      .select('*')
      .eq('email', email)
      .in('auth_method', ['email', 'linked'])
      .single();
    
    if (error || !user) {
      throw new Error('Invalid email or password');
    }
    
    const isValidPassword = await this.verifyPassword(password, user.password_hash);
    if (!isValidPassword) {
      throw new Error('Invalid email or password');
    }
    
    await this.updateLastLogin(user.id);
    delete user.password_hash;
    return user;
  }

  async getUserStats(userId) {
    const { data, error } = await supabase
      .from('profiles')
      .select(`
        level,
        experience,
        total_play_time,
        games_played,
        best_score,
        longest_survival_time,
        total_kills,
        total_deaths,
        total_damage_dealt,
        total_damage_taken,
        skill_points,
        coins
      `)
      .eq('id', userId)
      .single();
    
    if (error) {
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
          skill_id,
          name,
          description,
          category,
          icon,
          max_level,
          base_cost,
          cost_multiplier
        )
      `)
      .eq('profile_id', userId);
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async addUserSkill(userId, skillId, level = 0) {
    const { data, error } = await supabase
      .from('user_skills')
      .insert([{
        profile_id: userId,
        skill_id: skillId,
        level
      }])
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async upgradeUserSkill(userId, skillId) {
    // Get current skill level
    const { data: currentSkill } = await supabase
      .from('user_skills')
      .select('level')
      .eq('profile_id', userId)
      .eq('skill_id', skillId)
      .single();
    
    if (!currentSkill) {
      throw new Error('Skill not found for user');
    }
    
    const newLevel = currentSkill.level + 1;
    
    const { data, error } = await supabase
      .from('user_skills')
      .update({ level: newLevel })
      .eq('profile_id', userId)
      .eq('skill_id', skillId)
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async getUserInventory(userId) {
    const { data: cursors, error: cursorsError } = await supabase
        .from('user_cursors')
        .select('*')
      .eq('profile_id', userId);

    const { data: themes, error: themesError } = await supabase
        .from('user_themes')
        .select('*')
      .eq('profile_id', userId);
    
    if (cursorsError || themesError) {
      throw new Error('Failed to fetch user inventory');
    }
    
    return {
      cursors: cursors || [],
      themes: themes || []
    };
  }

  async addCursorToInventory(userId, cursorData) {
    const { data, error } = await supabase
      .from('user_cursors')
      .insert([{
        profile_id: userId,
        cursor_id: cursorData.cursorId,
        name: cursorData.name,
        rarity: cursorData.rarity
      }])
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async addThemeToInventory(userId, themeData) {
    const { data, error } = await supabase
      .from('user_themes')
      .insert([{
        profile_id: userId,
        theme_id: themeData.themeId,
        name: themeData.name
      }])
      .select()
      .single();
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data;
  }

  async getLeaderboard(type = 'score', limit = 10, timeframe = 'all') {
    let query = supabase
      .from('profiles')
      .select('username, level, avatar, best_score, longest_survival_time, total_kills')
      .eq('is_active', true);

    // Apply ordering based on type
    switch (type) {
      case 'survival':
        query = query.order('longest_survival_time', { ascending: false });
        break;
      case 'kills':
        query = query.order('total_kills', { ascending: false });
        break;
      default:
        query = query.order('best_score', { ascending: false });
    }
    
    const { data, error } = await query.limit(limit);
    
    if (error) {
      throw new Error(error.message);
    }
    
    return data.map((user, index) => ({
      rank: index + 1,
      username: user.username,
      level: user.level,
      avatar: user.avatar,
      score: user.best_score,
      survivalTime: user.longest_survival_time,
      kills: user.total_kills
    }));
  }

  async checkEmailExists(email) {
    const { data, error } = await supabase
      .from('profiles')
      .select('user_id')
      .eq('email', email)
      .single();
    
    if (error && error.code !== 'PGRST116') { // PGRST116 = not found
      throw new Error(error.message);
    }
    
    return !!data; // Return true if user found, false if not
  }

  async updateUserProgress(userId, progressData) {
    const { 
      leaderboard_points, 
      skill_points, 
      current_score, 
      survival_time 
    } = progressData;

    // Get current user data
    const { data: currentUser } = await supabase
      .from('profiles')
      .select('best_score, skill_points, longest_survival_time, total_play_time')
      .eq('id', userId)
      .single();

    if (!currentUser) {
      throw new Error('User not found');
    }

    // Calculate updates
    const updates = {
      skill_points: (currentUser.skill_points || 0) + (skill_points || 0),
      total_play_time: (currentUser.total_play_time || 0) + Math.floor(survival_time || 0),
      updated_at: new Date().toISOString()
    };

    // Update best score if current score is higher
    if (current_score && current_score > (currentUser.best_score || 0)) {
      updates.best_score = current_score;
    }

    // Update longest survival time if current time is longer
    if (survival_time && survival_time > (currentUser.longest_survival_time || 0)) {
      updates.longest_survival_time = Math.floor(survival_time);
    }

    const { data, error } = await supabase
      .from('profiles')
      .update(updates)
      .eq('id', userId)
      .select()
      .single();

    if (error) {
      throw new Error(error.message);
    }

    return data;
  }

  async endGameSession(userId, sessionData) {
    const {
      final_score,
      survival_time,
      kills = 0,
      damage_dealt = 0,
      damage_taken = 0,
      wave_reached = 1,
      leaderboard_points_earned = 0,
      skill_points_earned = 0
    } = sessionData;

    // Create game session record
    const { data: session, error: sessionError } = await supabase
      .from('game_sessions')
      .insert([{
        profile_id: userId,
        score: final_score,
        survival_time: Math.floor(survival_time),
        kills,
        damage_dealt,
        damage_taken,
        wave_reached,
        end_time: new Date().toISOString()
      }])
      .select()
      .single();

    if (sessionError) {
      throw new Error(sessionError.message);
    }

    // Update user stats
    const { data: currentUser } = await supabase
      .from('profiles')
      .select('*')
      .eq('id', userId)
      .single();

    if (!currentUser) {
      throw new Error('User not found');
    }

    const updates = {
      games_played: (currentUser.games_played || 0) + 1,
      skill_points: (currentUser.skill_points || 0) + skill_points_earned,
      total_play_time: (currentUser.total_play_time || 0) + Math.floor(survival_time),
      total_kills: (currentUser.total_kills || 0) + kills,
      total_damage_dealt: (currentUser.total_damage_dealt || 0) + damage_dealt,
      total_damage_taken: (currentUser.total_damage_taken || 0) + damage_taken,
      updated_at: new Date().toISOString()
    };

    // Update best score if this session was better
    if (final_score > (currentUser.best_score || 0)) {
      updates.best_score = final_score;
    }

    // Update longest survival time if this session was longer
    if (survival_time > (currentUser.longest_survival_time || 0)) {
      updates.longest_survival_time = Math.floor(survival_time);
    }

    const { data: updatedUser, error: updateError } = await supabase
      .from('profiles')
      .update(updates)
      .eq('id', userId)
      .select()
      .single();

    if (updateError) {
      throw new Error(updateError.message);
    }

    return {
      session,
      user: updatedUser
    };
  }

  async getUserById(userId) {
    const { data, error } = await supabase
      .from('profiles')
      .select('*')
      .eq('id', userId)
      .single();

    if (error) {
      throw new Error(error.message);
    }

    // Remove sensitive data
    delete data.password_hash;
    return data;
  }
}

module.exports = new UserService(); 