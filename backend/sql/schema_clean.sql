-- Desktop Survivor Dash - Clean Supabase Schema
-- This schema works with the authentication system using profile_id

-- Users table (main profiles table)
CREATE TABLE IF NOT EXISTS profiles (
  id bigint primary key generated always as identity,
  user_id uuid REFERENCES auth.users(id) ON DELETE CASCADE UNIQUE,
  username VARCHAR(255) UNIQUE,
  email VARCHAR(255) UNIQUE,
  steam_id VARCHAR(255) UNIQUE,
  avatar text DEFAULT 'default-cursor.png',
  auth_method VARCHAR(20) DEFAULT 'email' CHECK (auth_method IN ('email', 'steam', 'linked')),
  password_hash TEXT,
  level integer DEFAULT 1,
  experience integer DEFAULT 0,
  total_play_time integer DEFAULT 0,
  games_played integer DEFAULT 0,
  best_score integer DEFAULT 0,
  longest_survival_time integer DEFAULT 0,
  total_kills integer DEFAULT 0,
  total_deaths integer DEFAULT 0,
  total_damage_dealt integer DEFAULT 0,
  total_damage_taken integer DEFAULT 0,
  skill_points integer DEFAULT 0 CHECK (skill_points >= 0),
  coins integer DEFAULT 50 CHECK (coins >= 0),
  sound_enabled boolean DEFAULT true,
  music_enabled boolean DEFAULT true,
  volume integer DEFAULT 80 CHECK (volume >= 0 AND volume <= 100),
  graphics_quality text DEFAULT 'medium' CHECK (graphics_quality IN ('low', 'medium', 'high', 'ultra')),
  is_active boolean DEFAULT true,
  last_login timestamp with time zone DEFAULT NOW(),
  created_at timestamp with time zone DEFAULT NOW(),
  updated_at timestamp with time zone DEFAULT NOW()
);

-- Make user_id nullable (for game-only accounts)
ALTER TABLE profiles ALTER COLUMN user_id DROP NOT NULL;

-- Skills table
CREATE TABLE IF NOT EXISTS skills (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  skill_id VARCHAR(255) UNIQUE NOT NULL,
  name VARCHAR(255) NOT NULL,
  description TEXT NOT NULL,
  category VARCHAR(50) NOT NULL CHECK (category IN ('combat', 'survival', 'movement', 'utility', 'special')),
  icon VARCHAR(255),
  max_level INTEGER NOT NULL CHECK (max_level > 0),
  unlock_level INTEGER DEFAULT 1,
  base_cost INTEGER NOT NULL CHECK (base_cost > 0),
  cost_multiplier DECIMAL(3,2) DEFAULT 1.5,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Skill effects table
CREATE TABLE IF NOT EXISTS skill_effects (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  skill_id UUID REFERENCES skills(id) ON DELETE CASCADE,
  effect_type VARCHAR(50) NOT NULL CHECK (effect_type IN ('damage', 'health', 'speed', 'reload', 'range', 'critical', 'armor', 'experience', 'luck')),
  base_value DECIMAL(10,2) NOT NULL,
  per_level_increase DECIMAL(10,2) NOT NULL,
  is_percentage BOOLEAN DEFAULT false,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Skill prerequisites table
CREATE TABLE IF NOT EXISTS skill_prerequisites (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  skill_id UUID REFERENCES skills(id) ON DELETE CASCADE,
  required_skill_id UUID REFERENCES skills(id) ON DELETE CASCADE,
  required_level INTEGER DEFAULT 1,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- User skills table (using profile_id)
CREATE TABLE IF NOT EXISTS user_skills (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  skill_id UUID REFERENCES skills(id) ON DELETE CASCADE,
  level INTEGER DEFAULT 0 CHECK (level >= 0),
  experience INTEGER DEFAULT 0 CHECK (experience >= 0),
  unlocked_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  UNIQUE(profile_id, skill_id)
);

-- Game sessions table (using profile_id)
CREATE TABLE IF NOT EXISTS game_sessions (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  start_time TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  end_time TIMESTAMP WITH TIME ZONE,
  score INTEGER DEFAULT 0 CHECK (score >= 0),
  survival_time INTEGER DEFAULT 0 CHECK (survival_time >= 0),
  kills INTEGER DEFAULT 0 CHECK (kills >= 0),
  damage_dealt INTEGER DEFAULT 0 CHECK (damage_dealt >= 0),
  damage_taken INTEGER DEFAULT 0 CHECK (damage_taken >= 0),
  wave_reached INTEGER DEFAULT 1 CHECK (wave_reached >= 1),
  is_active BOOLEAN DEFAULT true,
  game_version VARCHAR(10) DEFAULT '1.0.0',
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- DLC items table
CREATE TABLE IF NOT EXISTS dlc_items (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  item_id VARCHAR(255) UNIQUE NOT NULL,
  name VARCHAR(255) NOT NULL,
  description TEXT NOT NULL,
  category VARCHAR(50) NOT NULL CHECK (category IN ('cursor', 'theme', 'avatar', 'effect')),
  rarity VARCHAR(20) DEFAULT 'common' CHECK (rarity IN ('common', 'rare', 'epic', 'legendary')),
  price DECIMAL(10,2) NOT NULL CHECK (price >= 0),
  currency VARCHAR(10) DEFAULT 'USD' CHECK (currency IN ('USD', 'EUR', 'GBP', 'STEAM')),
  preview_image VARCHAR(255),
  preview_video VARCHAR(255),
  preview_thumbnail VARCHAR(255),
  animation_frames INTEGER,
  resolution VARCHAR(20),
  file_size INTEGER,
  colors TEXT[],
  tags TEXT[],
  is_available BOOLEAN DEFAULT true,
  release_date TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  end_date TIMESTAMP WITH TIME ZONE,
  is_limited_edition BOOLEAN DEFAULT false,
  max_quantity INTEGER,
  sold_quantity INTEGER DEFAULT 0,
  featured BOOLEAN DEFAULT false,
  featured_order INTEGER DEFAULT 0,
  season VARCHAR(20) CHECK (season IN ('spring', 'summer', 'autumn', 'winter', 'halloween', 'christmas', 'valentine')),
  season_year INTEGER,
  steam_workshop_id VARCHAR(255),
  created_by VARCHAR(255) DEFAULT 'Official',
  downloads INTEGER DEFAULT 0,
  rating_average DECIMAL(3,2) DEFAULT 0 CHECK (rating_average >= 0 AND rating_average <= 5),
  rating_count INTEGER DEFAULT 0,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- User inventory tables (using profile_id)
CREATE TABLE IF NOT EXISTS user_cursors (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  cursor_id VARCHAR(255) NOT NULL,
  name VARCHAR(255) NOT NULL,
  rarity VARCHAR(20) DEFAULT 'common' CHECK (rarity IN ('common', 'rare', 'epic', 'legendary')),
  purchased_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  UNIQUE(profile_id, cursor_id)
);

CREATE TABLE IF NOT EXISTS user_themes (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  theme_id VARCHAR(255) NOT NULL,
  name VARCHAR(255) NOT NULL,
  purchased_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  UNIQUE(profile_id, theme_id)
);

-- User achievements table (using profile_id)
CREATE TABLE IF NOT EXISTS user_achievements (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  achievement_id VARCHAR(255) NOT NULL,
  unlocked_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  UNIQUE(profile_id, achievement_id)
);

-- User upgrades table (using profile_id)
CREATE TABLE IF NOT EXISTS user_upgrades (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  upgrade_id VARCHAR(255) NOT NULL,
  unlocked_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  UNIQUE(profile_id, upgrade_id)
);

-- Session upgrades table
CREATE TABLE IF NOT EXISTS session_upgrades (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  session_id UUID REFERENCES game_sessions(id) ON DELETE CASCADE,
  upgrade_id VARCHAR(255) NOT NULL,
  name VARCHAR(255) NOT NULL,
  level INTEGER DEFAULT 1,
  acquired_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_profiles_username ON profiles(username);
CREATE INDEX IF NOT EXISTS idx_profiles_email ON profiles(email);
CREATE INDEX IF NOT EXISTS idx_profiles_steam_id ON profiles(steam_id);
CREATE INDEX IF NOT EXISTS idx_profiles_auth_method ON profiles(auth_method);
CREATE INDEX IF NOT EXISTS idx_profiles_level ON profiles(level);
CREATE INDEX IF NOT EXISTS idx_profiles_best_score ON profiles(best_score);

CREATE INDEX IF NOT EXISTS idx_skills_category ON skills(category);
CREATE INDEX IF NOT EXISTS idx_skills_skill_id ON skills(skill_id);

CREATE INDEX IF NOT EXISTS idx_user_skills_profile_id ON user_skills(profile_id);
CREATE INDEX IF NOT EXISTS idx_user_skills_skill_id ON user_skills(skill_id);

CREATE INDEX IF NOT EXISTS idx_game_sessions_profile_id ON game_sessions(profile_id);
CREATE INDEX IF NOT EXISTS idx_game_sessions_score ON game_sessions(score DESC);
CREATE INDEX IF NOT EXISTS idx_game_sessions_survival_time ON game_sessions(survival_time DESC);
CREATE INDEX IF NOT EXISTS idx_game_sessions_kills ON game_sessions(kills DESC);

CREATE INDEX IF NOT EXISTS idx_dlc_items_category ON dlc_items(category);
CREATE INDEX IF NOT EXISTS idx_dlc_items_featured ON dlc_items(featured);

CREATE INDEX IF NOT EXISTS idx_user_cursors_profile_id ON user_cursors(profile_id);
CREATE INDEX IF NOT EXISTS idx_user_themes_profile_id ON user_themes(profile_id);
CREATE INDEX IF NOT EXISTS idx_user_achievements_profile_id ON user_achievements(profile_id);
CREATE INDEX IF NOT EXISTS idx_user_upgrades_profile_id ON user_upgrades(profile_id);

-- Triggers for updated_at columns
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER update_profiles_updated_at BEFORE UPDATE ON profiles FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();
CREATE TRIGGER update_skills_updated_at BEFORE UPDATE ON skills FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();
CREATE TRIGGER update_game_sessions_updated_at BEFORE UPDATE ON game_sessions FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();
CREATE TRIGGER update_dlc_items_updated_at BEFORE UPDATE ON dlc_items FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Comments
COMMENT ON COLUMN profiles.auth_method IS 'Authentication method: email (email/password), steam (Steam only), linked (both email and Steam)';
COMMENT ON COLUMN profiles.password_hash IS 'Bcrypt hash of user password for email authentication';
COMMENT ON COLUMN profiles.user_id IS 'Reference to Supabase auth.users - null for game-only accounts'; 