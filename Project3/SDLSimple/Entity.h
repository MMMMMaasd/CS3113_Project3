enum AnimationDirection { LEFT, RIGHT, UP, DOWN };

class Entity
{
private:
    
    int m_walking[4][4]; // 4x4 array for walking animations
    
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec3 m_scale;
    glm::vec3 m_roatet_vec = glm::vec3(0.0f, 1.0f, 0.0f);
    
    float m_rotate_angle;
    
    glm::mat4 m_model_matrix;
    
    float     m_speed;
    float     m_width;
    float     m_height;

    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;
    
    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;
    
    int  *m_animation_indices = nullptr;
    float m_animation_time    = 0.0f;
    
public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, int walking[4][4], float animation_time,
           int animation_frames, int animation_index, int animation_cols,
           int animation_rows);
    Entity(GLuint texture_id, float speed); // Simpler constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id,
                                        int index);
    
    bool const check_collision(Entity *other) const;
    
    void update(float delta_time);
    void render(ShaderProgram *program);
    void update(float delta_time, Entity* collidable_entities, Entity** collidable_entities_second, int entity_count, int entity_count_second, bool& gameStatus, bool& ifLose, bool& ifWin);
    
    void normalise_movement() { m_movement = glm::normalize(m_movement); };
    
    void face_left()  { }
    void face_right() { }
    
    void accelerate_left() { m_acceleration.x -= 0.1f; };
    void accelerate_right() {  m_acceleration.x += 0.1f; };
    void accelerate_up() { m_acceleration.y = 0.2f; };
    void accelerate_down() {  m_acceleration.y = -0.2f; };

    // ————— GETTERS ————— //
    glm::vec3 const get_position()     const { return m_position; };
    glm::vec3 const get_velocity()     const { return m_velocity; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_movement()     const { return m_movement; };
    glm::vec3 const get_scale()      const { return m_scale;      }
    float const get_rotate_angle() const {return m_rotate_angle;     }
    GLuint    const get_texture_id() const { return m_texture_id; }
    float     const get_speed()      const { return m_speed;      }
    float       const get_width()      const { return m_width; }
    float       const get_height()      const { return m_height; }

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position) { m_acceleration = new_position; };
    void const set_acceleration_x(float new_x) { m_acceleration.x = new_x; };
    void const set_acceleration_y(float new_y) { m_acceleration.y = new_y; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_rotate_angle(float new_angle) { m_rotate_angle = new_angle; };
    void const set_scale(glm::vec3 new_scale)        { m_scale      = new_scale;        }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id;   }
    void const set_rotate_vec(glm::vec3 new_vec) { m_roatet_vec = new_vec; }
    
    void const set_width(float new_width) { m_width = new_width; }
    void const set_height(float new_height) { m_height = new_height; }
    void const set_speed(float new_speed)           { m_speed      = new_speed;        }
    void const set_animation_cols(int new_cols)     { m_animation_cols = new_cols;     }
    void const set_animation_rows(int new_rows)     { m_animation_rows = new_rows;     }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index)   { m_animation_index = new_index;   }
    void const set_animation_time(int new_time)     { m_animation_time = new_time;     }

    // Setter for m_walking
    void set_walking(int walking[4][4])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m_walking[i][j] = walking[i][j];
            }
        }
    }
};
