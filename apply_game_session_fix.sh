#!/bin/bash

echo "🔧 Applying Game Session Fix..."
echo ""

# Check if .env file exists
if [ ! -f backend/.env ]; then
    echo "❌ Error: backend/.env file not found!"
    echo "Please ensure you have configured your environment variables."
    exit 1
fi

# Load environment variables
source backend/.env

echo "📊 Step 1: Applying database migration..."
echo "This will add missing columns to the game_sessions table."
echo ""

# Extract database URL components
if [[ $DATABASE_URL =~ postgresql://([^:]+):([^@]+)@([^:]+):([^/]+)/(.+) ]]; then
    DB_USER="${BASH_REMATCH[1]}"
    DB_PASS="${BASH_REMATCH[2]}"
    DB_HOST="${BASH_REMATCH[3]}"
    DB_PORT="${BASH_REMATCH[4]}"
    DB_NAME="${BASH_REMATCH[5]}"
    
    echo "Connecting to database..."
    PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -f backend/sql/add_missing_game_sessions_columns.sql
    
    if [ $? -eq 0 ]; then
        echo "✅ Database migration applied successfully!"
    else
        echo "❌ Failed to apply database migration. Please check your database connection."
        exit 1
    fi
else
    echo "❌ Could not parse DATABASE_URL. Please run the migration manually:"
    echo "psql -h [your-host] -U [your-user] -d [your-database] -f backend/sql/add_missing_game_sessions_columns.sql"
    exit 1
fi

echo ""
echo "🔄 Step 2: Restarting backend server..."
echo "Please restart your backend server by running:"
echo "  cd backend && npm start"
echo ""

echo "🛠️  Step 3: Rebuilding frontend..."
cd frontend
make clean
make

if [ $? -eq 0 ]; then
    echo "✅ Frontend rebuilt successfully!"
else
    echo "❌ Failed to rebuild frontend. Please check the build errors."
    exit 1
fi

cd ..

echo ""
echo "✨ Game Session Fix Applied Successfully!"
echo ""
echo "You can now run the game with: ./run_game.sh"
echo ""
echo "Expected improvements:"
echo "  ✓ Game sessions will start without HTTP 400 errors"
echo "  ✓ Progress will be saved every 5 seconds"
echo "  ✓ All scores (normal, leaderboard, skill) will persist properly"
echo "  ✓ No more 'Cannot save progress - no active session' errors"
echo ""
echo "Happy gaming! 🎮" 