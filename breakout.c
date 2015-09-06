//
// breakout.c
//
// Computer Science 50
// Problem Set 3
//

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cs50.h>

// Stanford Portable Library
#include <spl/gevents.h>
#include <spl/gobjects.h>
#include <spl/gwindow.h>

// height and width of game's window in pixels
#define HEIGHT 500
#define WIDTH 400

// number of rows of bricks
#define ROWS 5

// number of columns of bricks
#define COLS 10

// height of bricks
#define BRICK_H 12

// radius of ball in pixels
#define RADIUS 10

// lives
#define LIVES 3

// paddle dimensions
#define PADDLE_H 12
#define PADDLE_W 75

// default velocity
#define VELOCITY 3.0

// prototypes
void initBricks(GWindow window);
GOval initBall(GWindow window);
GRect initPaddle(GWindow window);
GLabel initScoreboard(GWindow window);
void updateScoreboard(GWindow window, GLabel label, int points);
GObject detectCollision(GWindow window, GOval ball);
int sign(void);

int main(int argc, string argv[])
{
    if(argc > 2)
    {
        printf("ERROR 1\n");
        return 1;
    }
    
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);

    // instantiate bricks
    initBricks(window);

    // instantiate ball, centered in middle of window
    GOval ball = initBall(window);

    // instantiate paddle, centered at bottom of window
    GRect paddle = initPaddle(window);
    add(window, paddle);

    // instantiate scoreboard, centered in middle of window, just above ball
    GLabel label = initScoreboard(window);
    char s[10];  
    
    // number of bricks initially
    int bricks = COLS * ROWS;

    // number of lives initially
    int lives = LIVES;

    // number of points initially
    int points = 0;
    
    double velocity_x = (drand48()*(VELOCITY-1) + 1)*sign();
    double velocity_y = VELOCITY;  
    
    waitForClick();
    
    // keep playing until game over
    
    while (lives > 0 && bricks > 0)
    {   
        // for paddle movement 
        int paddle_W = getWidth(paddle);
        int paddle_Y = getY(paddle);

        if(argc == 2)
        {
            if(strcmp(argv[1], "GOD") == 0)
            {
                int ball_x = getX(ball) + RADIUS;
                
                if(ball_x <= paddle_W/2)
                    setLocation(paddle, 0, getY(paddle));
                else if(ball_x >= WIDTH - paddle_W/2)
                    setLocation(paddle, WIDTH - paddle_W, paddle_Y);
                else
                    setLocation(paddle, getX(ball) - paddle_W/2, paddle_Y);
            }
        }
        else if (argc == 1)
        { 
            GEvent event = getNextEvent(MOUSE_EVENT);
            if(event != NULL)
            {
                if(getEventType(event) == MOUSE_MOVED)
                {
                    double x = getX(event);
                    if(x <= paddle_W/2)
                    {
                        setLocation(paddle, 0, paddle_Y);
                    }
                    else if (x >= WIDTH - paddle_W/2)
                    {
                        setLocation(paddle, WIDTH - paddle_W, paddle_Y);
                    }
                    else
                    {
                        setLocation(paddle, x - paddle_W/2, paddle_Y);
                    }
                }
            }
        }
        // for ball movement
        move(ball, velocity_x, velocity_y);
        pause(10);
       
        // for ball containment between left, right, and top side
        if(getX(ball) <= 0 || getX(ball) >= WIDTH - RADIUS * 2)
        {
            velocity_x = -velocity_x;
        }
        else if(getY(ball) <= 0)
        {
            velocity_y = -velocity_y;
        }
        
        // for collision detection
        GObject object = detectCollision(window, ball);
        
        if(object != NULL)
        {
            if(strcmp(getType(object), "GRect") == 0)
            {   
                // when ball collides with paddle
                if(object == paddle)
                {                       
                    // if ball hits side of paddle, it continues to move downwards
                    if(getY(ball) + RADIUS >= HEIGHT - PADDLE_H)
                    {
                        velocity_x *= -1;
                    }
                    // otherwise, ball moves upwards
                    else
                    {
                        velocity_y *= -1;
                        
                        int ballPosition = getX(ball) + RADIUS;
                        int paddle_25 = getX(paddle) + paddle_W * 0.25;
                        int paddle_75 = getX(paddle) + paddle_W* 0.75;
                        
                        if(velocity_x < 0)
                        {
                            if(ballPosition < paddle_25)
                                velocity_x *= 1.15;
                            else if(ballPosition >= paddle_25 && ballPosition < paddle_75)
                                velocity_x = -VELOCITY;
                            else if(ballPosition >= paddle_75)
                                velocity_x *= -1.15;
                        }
                        else
                        {
                            if(ballPosition < paddle_25)
                                velocity_x *= -1.15;
                            else if(ballPosition >= paddle_25 &cd& ballPosition < paddle_75)
                                velocity_x = VELOCITY;
                            else if(ballPosition >= paddle_75)
                                velocity_x *= 1.15; 
                        }
                    }
                }
                   
                // if ball collides with brick             
                else
                {
                    if(getY(ball) < getY(object) + getHeight(object))
                    {
                        velocity_x *= -1;
                    }
                    
                    setSize(paddle, getWidth(paddle)*0.98, PADDLE_H);
                    removeGWindow(window, object);
                    velocity_y *= -1;
                    points++;
                    bricks--;
                    
                    sprintf(s, "Score: %i", points);
                    setLabel(label, s); 
                    setLocation(label, (WIDTH - getWidth(label))/2, (HEIGHT - getHeight(label))/2);
    
                }
            }       
        }
        
        if(getY(ball) > HEIGHT)
        {
            lives--;
            
            if(lives > 0)
            {
                setLocation(ball, WIDTH/2 - RADIUS, HEIGHT/2 - RADIUS);
                waitForClick();
            }
        }
    }
    char t[10];
    GLabel winLose = newGLabel("");
    setFont(winLose, "SanSerif-36");
    setColor(winLose, "RED");
    add(window, winLose);
    if(lives == 0)
        sprintf(t,"%s", "You Lose!");
    else if(lives > 0)
        sprintf(t,"%s", "You Win!");
    setLabel(winLose, t);        
    setLocation(winLose, (WIDTH - getWidth(winLose))/2, (HEIGHT - getHeight(winLose))/2 + getHeight(label));

    // wait for click before exiting
    waitForClick();

    // game over
    closeGWindow(window);
    return 0;
}

/**
 * Initializes window with a grid of bricks.
 */
 
int sign(void)
{
    if(drand48() < 0.5)
        return -1;
    else
        return 1;
}
 
void initBricks(GWindow window)
{
    for(int i = 0; i < ROWS; i++)
    {
        for(int j = 0; j < COLS; j++)
        {
            GRect brick = newGRect(j*(WIDTH/COLS) + 1, i*(BRICK_H) + i*2, WIDTH/COLS-2, BRICK_H);
            add(window, brick);
            
            switch (i)  
            {
                case 0:
                    setColor(brick, "RED");
                    setFilled(brick, true);
                    break;
                case 1:
                    setColor(brick, "ORANGE");
                    setFilled(brick, true);
                    break;
                case 2:
                    setColor(brick, "YELLOW");
                    setFilled(brick, true);
                    break;
                case 3:
                    setColor(brick, "GREEN");
                    setFilled(brick, true);
                    break;
                case 4:
                    setColor(brick, "BLUE");
                    setFilled(brick, true);
                    break;
                default:
                    break;
            }
        }
    }
}

/**
 * Instantiates ball in center of window.  Returns ball.
 */
GOval initBall(GWindow window)
{
    GOval ball = newGOval(WIDTH/2 - RADIUS, HEIGHT/2 - RADIUS, RADIUS*2, RADIUS*2);
    setColor(ball, "BLUE");
    setFilled(ball, true);
    add(window, ball);
    return ball;
}

/**
 * Instantiates paddle in bottom-middle of window.
 */
GRect initPaddle(GWindow window)
{
    GRect paddle = newGRect(0,0, PADDLE_W, PADDLE_H);
    setColor(paddle, "BLACK");
    setFilled(paddle, true);
    
    double x = (getWidth(window) - getWidth(paddle))/2;
    double y = getHeight(window) - getHeight(paddle);
    setLocation(paddle, x, y);
    
    return paddle;
}

/**
 * Instantiates, configures, and returns label for scoreboard.
 */
GLabel initScoreboard(GWindow window)
{
    GLabel score = newGLabel("");
    setFont(score, "SansSerif-36");
    setColor(score, "BLACK");
    add(window, score);
    setLocation(score, (WIDTH - getWidth(score))/2, (HEIGHT - getHeight(score))/2);
    return score;
}

/**
 * Updates scoreboard's label, keeping it centered in window.
 */
void updateScoreboard(GWindow window, GLabel label, int points)
{
    // update label
    char s[3];
    sprintf(s, "%i", points);
    setLabel(label, s);

    // center label in window
    double x = (getWidth(window) - getWidth(label)) / 2;
    double y = (getHeight(window) - getHeight(label)) / 2;
    setLocation(label, x, y);
}

/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
GObject detectCollision(GWindow window, GOval ball)
{
    // ball's location
    double x = getX(ball);
    double y = getY(ball);

    // for checking for collisions
    GObject object;

    // check for collision at ball's top-left corner
    object = getGObjectAt(window, x, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // no collision
    return NULL;
}
