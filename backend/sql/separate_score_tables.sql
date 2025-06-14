-- Separate Score Tables Migration
-- Replace views with actual tables for proper data storage

-- Drop existing views
DROP VIEW IF EXISTS leaderboard_by_score;
DROP VIEW IF EXISTS leaderboard_by_leaderboard_points;
DROP VIEW IF EXISTS leaderboard_by_survival;

-- 1. NORMAL SCORES TABLE - Individual game session high scores
CREATE TABLE IF NOT EXISTS normal_scores (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  session_id UUID REFERENCES game_sessions(id) ON DELETE CASCADE,
  score integer NOT NULL CHECK (score >= 0),
  survival_time integer NOT NULL CHECK (survival_time >= 0),
  kills integer DEFAULT 0 CHECK (kills >= 0),
  enemies_spawned integer DEFAULT 0 CHECK (enemies_spawned >= 0),
  damage_dealt integer DEFAULT 0 CHECK (damage_dealt >= 0),
  damage_taken integer DEFAULT 0 CHECK (damage_taken >= 0),
  wave_reached integer DEFAULT 1 CHECK (wave_reached >= 1),
  achieved_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  is_personal_best BOOLEAN DEFAULT false,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 2. LEADERBOARD SCORES TABLE - Accumulated leaderboard points
CREATE TABLE IF NOT EXISTS leaderboard_scores (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  session_id UUID REFERENCES game_sessions(id) ON DELETE CASCADE,
  leaderboard_points integer NOT NULL CHECK (leaderboard_points >= 0),
  points_earned_this_session integer NOT NULL CHECK (points_earned_this_session >= 0),
  total_accumulated_points integer NOT NULL CHECK (total_accumulated_points >= 0),
  survival_time integer NOT NULL CHECK (survival_time >= 0),
  earning_rate DECIMAL(5,2) DEFAULT 2.0, -- Points per second rate
  session_start_time TIMESTAMP WITH TIME ZONE,
  session_end_time TIMESTAMP WITH TIME ZONE,
  achieved_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 3. SKILL SCORES TABLE - Skill points for purchasing upgrades
CREATE TABLE IF NOT EXISTS skill_scores (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  session_id UUID REFERENCES game_sessions(id) ON DELETE CASCADE,
  skill_points integer NOT NULL CHECK (skill_points >= 0),
  points_earned_this_session integer NOT NULL CHECK (points_earned_this_session >= 0),
  total_accumulated_points integer NOT NULL CHECK (total_accumulated_points >= 0),
  points_spent integer DEFAULT 0 CHECK (points_spent >= 0),
  points_available integer NOT NULL CHECK (points_available >= 0),
  survival_time integer NOT NULL CHECK (survival_time >= 0),
  earning_rate DECIMAL(5,2) DEFAULT 1.0, -- Points per second rate
  last_spent_on VARCHAR(255), -- Track what skill points were spent on
  session_start_time TIMESTAMP WITH TIME ZONE,
  session_end_time TIMESTAMP WITH TIME ZONE,
  achieved_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Add indexes for performance
CREATE INDEX IF NOT EXISTS idx_normal_scores_profile_id ON normal_scores(profile_id);
CREATE INDEX IF NOT EXISTS idx_normal_scores_score ON normal_scores(score DESC);
CREATE INDEX IF NOT EXISTS idx_normal_scores_survival_time ON normal_scores(survival_time DESC);
CREATE INDEX IF NOT EXISTS idx_normal_scores_achieved_at ON normal_scores(achieved_at DESC);

CREATE INDEX IF NOT EXISTS idx_leaderboard_scores_profile_id ON leaderboard_scores(profile_id);
CREATE INDEX IF NOT EXISTS idx_leaderboard_scores_total_points ON leaderboard_scores(total_accumulated_points DESC);
CREATE INDEX IF NOT EXISTS idx_leaderboard_scores_achieved_at ON leaderboard_scores(achieved_at DESC);

CREATE INDEX IF NOT EXISTS idx_skill_scores_profile_id ON skill_scores(profile_id);
CREATE INDEX IF NOT EXISTS idx_skill_scores_total_points ON skill_scores(total_accumulated_points DESC);
CREATE INDEX IF NOT EXISTS idx_skill_scores_available ON skill_scores(points_available DESC);
CREATE INDEX IF NOT EXISTS idx_skill_scores_achieved_at ON skill_scores(achieved_at DESC);

-- Add triggers for updated_at columns
CREATE TRIGGER update_normal_scores_updated_at 
BEFORE UPDATE ON normal_scores 
FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_leaderboard_scores_updated_at 
BEFORE UPDATE ON leaderboard_scores 
FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_skill_scores_updated_at 
BEFORE UPDATE ON skill_scores 
FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Add comments for clarity
COMMENT ON TABLE normal_scores IS 'Individual game session scores for high score records';
COMMENT ON TABLE leaderboard_scores IS 'Accumulated leaderboard points for overall ranking';
COMMENT ON TABLE skill_scores IS 'Skill points for purchasing upgrades and abilities';

COMMENT ON COLUMN normal_scores.score IS 'Game session score calculated during gameplay';
COMMENT ON COLUMN normal_scores.is_personal_best IS 'Whether this is the players personal best score';

COMMENT ON COLUMN leaderboard_scores.total_accumulated_points IS 'Total leaderboard points accumulated across all sessions';
COMMENT ON COLUMN leaderboard_scores.points_earned_this_session IS 'Points earned in this specific session';

COMMENT ON COLUMN skill_scores.total_accumulated_points IS 'Total skill points earned across all sessions';
COMMENT ON COLUMN skill_scores.points_available IS 'Skill points available to spend (total - spent)';
COMMENT ON COLUMN skill_scores.points_spent IS 'Total skill points spent on upgrades';

-- Update profiles table to track current totals (for quick access)
ALTER TABLE profiles ADD COLUMN IF NOT EXISTS current_leaderboard_points integer DEFAULT 0 CHECK (current_leaderboard_points >= 0);
ALTER TABLE profiles ADD COLUMN IF NOT EXISTS current_skill_points integer DEFAULT 0 CHECK (current_skill_points >= 0);
ALTER TABLE profiles ADD COLUMN IF NOT EXISTS highest_score integer DEFAULT 0 CHECK (highest_score >= 0);

-- Create indexes for the new profile columns
CREATE INDEX IF NOT EXISTS idx_profiles_current_leaderboard_points ON profiles(current_leaderboard_points DESC);
CREATE INDEX IF NOT EXISTS idx_profiles_current_skill_points ON profiles(current_skill_points DESC);
CREATE INDEX IF NOT EXISTS idx_profiles_highest_score ON profiles(highest_score DESC);

-- Add comments for new profile columns
COMMENT ON COLUMN profiles.current_leaderboard_points IS 'Current total leaderboard points (for quick leaderboard queries)';
COMMENT ON COLUMN profiles.current_skill_points IS 'Current available skill points (total earned - spent)';
COMMENT ON COLUMN profiles.highest_score IS 'Highest individual game score achieved (personal best)'; 