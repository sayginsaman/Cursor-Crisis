# Desktop Survivor Dash Backend

## Supabase Migration Complete ✅

This backend has been migrated from MongoDB to Supabase (PostgreSQL) for better scalability and management.

## Setup Instructions

### 1. Create a Supabase Project
1. Go to [supabase.com](https://supabase.com) and create a new project
2. Note down your project URL and anon key from the project settings

### 2. Set up the Database Schema
1. In your Supabase dashboard, go to the SQL Editor
2. Copy and paste the contents of `sql/schema.sql` to create all tables
3. Run the SQL to create the database structure

### 3. Environment Variables
Create a `.env` file in the backend directory with:

```bash
# Server Configuration
NODE_ENV=development
PORT=3000

# JWT Configuration
JWT_SECRET=your_super_secret_jwt_key_here_make_it_long_and_random
JWT_EXPIRES_IN=7d

# Supabase Configuration
SUPABASE_URL=https://your-project-ref.supabase.co
SUPABASE_ANON_KEY=your_supabase_anon_key_here

# Optional: For admin operations
SUPABASE_SERVICE_ROLE_KEY=your_supabase_service_role_key_here

# CORS Configuration
CORS_ORIGIN=http://localhost:8080
```

### 4. Install Dependencies
```bash
npm install
```

### 5. Start the Server
```bash
npm start
```

## Key Changes from MongoDB

### Database Structure
- **Users**: Now stored in PostgreSQL with proper constraints and indexes
- **Skills**: Normalized into separate tables (skills, skill_effects, skill_prerequisites)
- **User Skills**: Separate junction table for user skill progress
- **Game Sessions**: Dedicated table with foreign key relationships
- **DLC Items**: Enhanced with better categorization and metadata

### API Endpoints
All existing endpoints work the same, but now use:
- Supabase client instead of Mongoose
- PostgreSQL queries instead of MongoDB operations
- UUID primary keys instead of ObjectIds
- Proper relational joins instead of embedded documents

### Benefits
- **Better Performance**: PostgreSQL with proper indexes
- **ACID Compliance**: True transactions and data integrity
- **Real-time Features**: Built-in real-time subscriptions
- **Better Queries**: Complex joins and aggregations
- **Scalability**: Automatic scaling and connection pooling
- **Security**: Row Level Security (RLS) policies

## Row Level Security (RLS)

You should set up RLS policies in Supabase for security:

1. Go to Authentication > Policies in your Supabase dashboard
2. Enable RLS on all tables
3. Create policies for user data access

Example policy for users table:
```sql
CREATE POLICY "Users can view own profile" ON users 
  FOR SELECT USING (auth.uid() = id);
  
CREATE POLICY "Users can update own profile" ON users 
  FOR UPDATE USING (auth.uid() = id);
```

## Available Scripts

- `npm start` - Start the server in production mode
- `npm run dev` - Start the server with nodemon for development
- `npm test` - Run tests (when implemented)

## API Documentation

The API endpoints remain the same as the MongoDB version:

- `POST /api/auth/register` - Register new user
- `POST /api/auth/login` - Login user
- `GET /api/auth/me` - Get current user
- `GET /api/skills` - Get all skills
- `GET /api/skills/user` - Get user's skills with progress
- `POST /api/skills/upgrade` - Upgrade a skill
- `GET /api/skills/categories` - Get skills by category
- `POST /api/game/start` - Start new game session
- `PUT /api/game/end` - End game session
- `GET /api/leaderboard` - Get leaderboards

## Error Handling

The API uses consistent error responses:

```json
{
  "success": false,
  "error": "Error message here",
  "errors": [] // For validation errors
}
```

## Contributing

When making changes:
1. Update the schema in `sql/schema.sql` if needed
2. Update corresponding service methods
3. Test all endpoints with the new database structure
4. Update this README if needed 