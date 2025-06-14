-- Add missing columns to game_sessions table for proper game session tracking

-- Add game_mode column
ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS game_mode VARCHAR(50) DEFAULT 'normal' CHECK (game_mode IN ('normal', 'hardcore', 'tutorial', 'custom'));

-- Add status column
ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS status VARCHAR(20) DEFAULT 'in_progress' CHECK (status IN ('in_progress', 'completed', 'abandoned'));

-- Add point tracking columns
ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS leaderboard_points_earned INTEGER DEFAULT 0 CHECK (leaderboard_points_earned >= 0);

ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS skill_points_earned INTEGER DEFAULT 0 CHECK (skill_points_earned >= 0);

-- Add detailed timing columns
ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS started_at TIMESTAMP WITH TIME ZONE DEFAULT NOW();

ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS ended_at TIMESTAMP WITH TIME ZONE;

-- Add end reason tracking
ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS end_reason VARCHAR(50) CHECK (end_reason IN ('player_death', 'player_quit', 'session_timeout', 'game_completed'));

-- Add enemies spawned column
ALTER TABLE game_sessions 
ADD COLUMN IF NOT EXISTS enemies_spawned INTEGER DEFAULT 0 CHECK (enemies_spawned >= 0);

-- Add snapshot tables for progress tracking
CREATE TABLE IF NOT EXISTS game_progress_snapshots (
  id UUID DEFAULT gen_random_uuid() PRIMARY KEY,
  session_id UUID REFERENCES game_sessions(id) ON DELETE CASCADE,
  profile_id bigint REFERENCES profiles(id) ON DELETE CASCADE,
  score INTEGER DEFAULT 0,
  leaderboard_points INTEGER DEFAULT 0,
  skill_points INTEGER DEFAULT 0,
  survival_time FLOAT DEFAULT 0,
  kills INTEGER DEFAULT 0,
  enemies_spawned INTEGER DEFAULT 0,
  damage_dealt INTEGER DEFAULT 0,
  damage_taken INTEGER DEFAULT 0,
  wave_reached INTEGER DEFAULT 1,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Add indexes for performance
CREATE INDEX IF NOT EXISTS idx_game_sessions_status ON game_sessions(status);
CREATE INDEX IF NOT EXISTS idx_game_sessions_game_mode ON game_sessions(game_mode);
CREATE INDEX IF NOT EXISTS idx_game_sessions_started_at ON game_sessions(started_at DESC);
CREATE INDEX IF NOT EXISTS idx_game_progress_snapshots_session_id ON game_progress_snapshots(session_id);
CREATE INDEX IF NOT EXISTS idx_game_progress_snapshots_profile_id ON game_progress_snapshots(profile_id); 