#include<windows.h>
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>
#include<iostream>
#include<string>
#include<algorithm>
#include <bits/stdc++.h>
using namespace std;

const double pi=3.14156;
const double eps = 1E-9;
const double Win_width = 700, Win_height = 700;
double red[]={1, 0, 0};
double green[]={0, 1, 0};
double blue[]={0, 0, 1};
double radius = 0.1;
double R[] = {-5*radius, 0, 5*radius};
double C[] = {-5*radius, 0, 5*radius};
int k=0, NOW=0;
double X, Y;
int refreshMills = 30; // refresh interval in milliseconds
bool turn = 0;
int clicked = -1;
int prv = -1;
bool adj[10][9] = {false};
int RR[] = {0, 1, 0, -1, -1, 1, 1, -1};
int CC[] = {1, 0, -1, 0, 1, 1, -1, -1};
int score[2]={0};

struct pt
{
    double x, y;
    pt(double x=0, double y=0)
    {
        this->x=x; this->y=y;
    }

    double len(const pt &a)
    {
        return sqrt((a.x-x)*(a.x-x) + (a.y-y)*(a.y-y));
    }

    bool operator == (const pt &a)
    {
        return (fabs(x-a.x)<eps && fabs(y-a.y)<eps);
    }

    pt operator + (const pt &a)
    {
        return pt(x+a.x, y+a.y);
    }

    pt operator - (const pt &a)
    {
        return pt(x-a.x, y-a.y);
    }

    pt operator * (const double &a)
    {
        return pt(a*x, a*y);
    }
};

struct circle
{
    pt c, tr, target;
    double r;
    int status;

    circle(pt c=pt(0,0), double r=0.1, int status=0, pt tr=pt(0, 0))
    {
        this->c=c; this->r=r; this->status=status; this->tr=tr;
        //draw();
    }

    bool is_outside(pt a)
    {
        return (((a.x-c.x)*(a.x-c.x) + (a.y-c.y)*(a.y-c.y) - r*r) > eps);
    }

    void draw()
    {
        int i;
        int triangleAmount = 100; //# of triangles used to draw circle

        //double radius = 0.8f; //radius
        double twicePi = 2.0f * pi;
        glBegin(GL_LINE_LOOP);
        for(i = 0; i <= triangleAmount;i++) {
            glVertex2f(
                    c.x + (r * cos(i *  twicePi / triangleAmount)),
                c.y + (r * sin(i * twicePi / triangleAmount))
            );
        }

        glEnd();
    }

    void fill_up()
    {
        int i;
        int triangleAmount = 100; //# of triangles used to draw circle

        //double radius = 0.8f; //radius
        double twicePi = 2.0f * pi;
        glBegin(GL_TRIANGLE_FAN), glColor3f(1.0f, 0.0f, 0.0f), glVertex2f(c.x, c.y); // center of circle

        if(status == 0) glColor3f(0.0f, 1.0f, 0.0f);
        else if(status == 1) glColor3f(0.0f, 0.0f, 1.0f);
        for(i = 0; i <= triangleAmount;i++) {
            glVertex2f(
                    c.x + (r * cos(i *  twicePi / triangleAmount)),
                c.y + (r * sin(i * twicePi / triangleAmount))
            );
        }

        glEnd();
    }
} pos[3][3], cir[2][3];

struct rectangle
{
    pt c, tr;
    double a, b;

    rectangle(pt c=pt(1.5, 0), double a=0.1, double b=0.1, pt tr=pt(0, 0))
    {
        this->c=c; this->a=a; this->b=b; this->tr=tr;
    }

    void draw(double R, double G, double B)
    {
        glBegin(GL_QUADS);
            glColor3f(R, G, B);
            glVertex2f(c.x+a/2.0, c.y+b/2.0);
            glVertex2f(c.x-a/2.0, c.y+b/2.0);
            glVertex2f(c.x-a/2.0, c.y-b/2.0);
            glVertex2f(c.x+a/2.0, c.y-b/2.0);
        glEnd();
    }
} sel_box, SCORE_A, SCORE_B, EXIT, RESTART;

bool is_not_outside(int x, int y) { return (x>=0 && y>=0 && x<3 && y<3); }
void init_board()
{
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            pos[i][j] = circle(pt(R[i], C[j]), radius, -1);//, pos[i][j].draw();
            //for(int kk=0; kk<10000000; kk++);
        }
        cir[0][i] = circle(pt(0, 0), radius, 0, pt(R[i], C[0]-0.24f));
        cir[0][i].target = pt(R[i], C[0]-0.24f);
        cir[1][i] = circle(pt(0, 0), radius, 1, pt(R[i], C[2]+0.24f));
        cir[1][i].target = pt(R[i], C[2]+0.24f);
    }
    sel_box = rectangle(pt(0, 0), 0.25, 0.25, pt(5.5, 0));
    SCORE_B = rectangle(pt(0, 0), 0.3, 0.1, pt(0.25, 0.95));
    SCORE_A = rectangle(pt(0, 0), 0.3, 0.1, pt(-0.25, 0.95));
    EXIT = rectangle(pt(0, 0), 0.4, 0.1, pt(0.87, 0.95));
    RESTART = rectangle(pt(0, 0), 0.4, 0.1, pt(0.87, 0.85));

    turn = 0; clicked = -1; prv = 9; k=0;//score[0]=score[1]=0;
    memset(adj, 0, sizeof(adj));
    for(int i=0; i<9; i++)
    {
        adj[9][i] = 1;
        for(int j=0; j<4; j++) if(is_not_outside((i/3)+RR[j], (i%3)+CC[j]))
            adj[i][3*((i/3)+RR[j]) + (i%3) + CC[j]] = 1;
        for(int j=4; i%2==0 && j<8; j++) if(is_not_outside((i/3)+RR[j], (i%3)+CC[j]))
            adj[i][3*((i/3)+RR[j]) + (i%3) + CC[j]] = 1;
    }
}

void createBoard()
{
    //glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(3.0);
    glColor3f(1.0f, 1.0f, 1.0f);     // White

//            pos[0][0] = circle(R[0], C[0], radius, 0);
    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
            {pos[i][j].draw(); }

    for(int i=0; i<3; i++)
        for(int j=1; j<3; j++)
        {
            glBegin(GL_LINES);
                glVertex2f(R[j-1]+radius, C[i]); glVertex2f(R[j]-radius, C[i]);
                glVertex2f(R[i], C[j-1]+radius); glVertex2f(R[i], C[j]-radius);
            glEnd();
        }

    double dx = radius/sqrt(2.0);
    glBegin(GL_LINES);
        glVertex2f(R[0]+dx, C[0]+dx); glVertex2f(R[1]-dx, C[1]-dx);
        glVertex2f(R[1]+dx, C[1]+dx); glVertex2f(R[2]-dx, C[2]-dx);
        glVertex2f(R[2]-dx, C[0]+dx); glVertex2f(R[1]+dx, C[1]-dx);
        glVertex2f(R[1]-dx, C[1]+dx); glVertex2f(R[0]+dx, C[2]-dx);
    glEnd();

    //glFlush();
}

// Global variable
GLfloat angle = 0.0f;  // Current rotational angle of the shapes

/* Initialize OpenGL Graphics */
void initGL() {
   // Set "clearing" or background color
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black and opaque
}

/* Called back when timer expired */
void Timer(int value) {
   glutPostRedisplay();      // Post re-paint request to activate display()
   glutTimerFunc(refreshMills, Timer, 0); // next Timer call milliseconds later
}

/* Callback handler for mouse event */
void start(int button, int state, int x, int y) {
    double xx, yy;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // whether select a position
        //if(NOW)
        X = xx = -1.0 + 2.0*x/Win_width, Y = yy = 1.0 - 2.0*y/Win_height;
                    cout << "x = " << xx << " y = " << yy << endl;
        if(xx+eps>0.87-0.2 && xx<eps+0.87+0.2
           && yy+eps>0.95-0.05 && xx<eps+0.95+0.05)
            exit(0);
        if(xx+eps>0.87-0.2 && xx<eps+0.87+0.2
           && yy+eps>0.85-0.05 && xx<eps+0.85+0.05)
            { init_board(); return;}
        //if(clicked == -1)
            for(int i=0; i<3; i++)
            {
                if(!cir[turn][i].is_outside(pt(xx, yy)))
                {
                    if(k<6 && (yy + eps > -0.6 && yy < eps + 0.6))
                        return;
                    //pos[i/3][i%3].status = turn+1;
                    sel_box.tr = cir[turn][i].c;
                    clicked = i;

                    prv = 9;
                    for(int i1=0; i1<9; i1++)
                        if(!pos[i1/3][i1%3].is_outside(pt(xx, yy)))
                            prv = i1;

                    return;
                }
            }

            for(int i=0; i<3 && clicked!=-1; i++)
            {
                for(int j=0; j<3 && clicked!=-1; j++)
                {
                    if(!pos[i][j].is_outside(pt(xx, yy)) && pos[i][j].status == -1 && adj[prv][3*i+j])
                    {
                        sel_box.tr = pt(1.5, 0);
                        cout << "cir[0][0] = {" << cir[0][0].c.x << ", " << cir[0][0].c.y << "}" << endl;
                        cir[turn][clicked].target = pos[i][j].c;
                        cir[turn][clicked].tr = (cir[turn][clicked].target - cir[turn][clicked].c)*(1.0/10.0);
                        cout << "===> " << cir[turn][clicked].tr.x << ", "<< cir[turn][clicked].tr.y << endl;
                        pos[i][j].status = turn;
                        if(prv<9) pos[prv/3][prv%3].status = -1;
                        clicked = -1, turn = 1 - turn;
                        k++;
                        //break;
                    }
                }
            }
    }
}

bool is_equal(int a, int b, int c) { return (a==b && a==c && a!=-1); }

void is_game_over()
{
    for(int i=0; i<3; i++)
    {
        if(is_equal(pos[i][0].status, pos[i][1].status, pos[i][2].status))
            score[pos[i][0].status]++, init_board();
        if(is_equal(pos[0][i].status, pos[1][i].status, pos[2][i].status))
            score[pos[0][i].status]++, init_board();
    }
    if(is_equal(pos[0][0].status, pos[1][1].status, pos[2][2].status))
        score[pos[0][0].status]++, init_board();
    if(is_equal(pos[2][0].status, pos[1][1].status, pos[0][2].status))
        score[pos[2][0].status]++, init_board();
}

string num_to_str(int a)
{
    if(a==0) return "0";
    int cnt=0; string res1 = "", res="";
    while(a) res1+=48+a%10, a/=10, cnt++;
    while(cnt--) res+=res1[cnt];
    return res;
}

void displayText( float x, float y, int r, int g, int b, string str ) {
	int j = str.length();//strlen( string );

	glColor3f( r, g, b );
	glRasterPos2f( x, y );
	for( int i = 0; i < j; i++ ) {
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, str[i] );
	}
}

/* Handler for window-repaint event. Call back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
    if(!NOW)
    {
        init_board();
    }
    //cout << NOW << endl;
    glClear(GL_COLOR_BUFFER_BIT);   // Clear the color buffer
    glMatrixMode(GL_MODELVIEW);     // To operate on Model-View matrix
    glLoadIdentity();               // Reset the model-view matrix

    glPushMatrix();                     // Save model-view matrix setting
        createBoard();
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(EXIT.tr.x, EXIT.tr.y, 0.0);
        EXIT.draw(0.91, 0.07, 0.14);
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(RESTART.tr.x, RESTART.tr.y, 0.0);
        RESTART.draw(0.47, 0.10, 0.67);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(sel_box.tr.x, sel_box.tr.y, 0.0);
        sel_box.draw(0.7, 0.7, 0.7);//(0.96f, 0.71f, 0.0f);
    glPopMatrix();
    //cout<< num_to_str(345)<<endl;
    displayText(-0.32, 0.93, 0, 1, 0, "A: "+ num_to_str(score[0]));
    displayText(0.2, 0.93, 0, 0, 1, "B: "+ num_to_str(score[1]));
    displayText(0.76, 0.93, 1, 1, 1, "EXIT");
    displayText(0.68, 0.83, 1, 1, 1, "RESTART");

    for(int i=0; i<2; i++)
        for(int j=0; j<3; j++)
        {
            glPushMatrix();                     // Save model-view matrix setting
                glTranslatef(cir[i][j].tr.x, cir[i][j].tr.y, 0.0);
                cir[i][j].fill_up();
            glPopMatrix();
        }

    glutSwapBuffers();   // Double buffered - swap the front and back buffers

    NOW++;

    int cnt=0;
    for(int i=0; i<2; i++)
        for(int j=0; j<3; j++)
        {
            // cout << cir[i][j].tr.len(pt(0, 0)) << "\t" << cir[i][j].c.len(cir[i][j].target) << endl;
            if(cir[i][j].tr.len(pt(0, 0)) + eps < cir[i][j].c.len(cir[i][j].target))
                cir[i][j].c = cir[i][j].c + cir[i][j].tr;
            else
            {
                cir[i][j].c = cir[i][j].target;
                cir[i][j].tr = pt(0, 0);
                cnt++;
            }
        }

    if(cnt==6 && k >= 5) is_game_over();



    // Change the rotational angle after each display()
    //angle += 0.2f;
}
/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
   // Compute aspect ratio of the new window
   if (height == 0) height = 1;                // To prevent divide by 0
   GLfloat aspect = (GLfloat)width / (GLfloat)height;

   // Set the viewport to cover the new window
   glViewport(0, 0, width, height);

   // Set the aspect ratio of the clipping area to match the viewport
   glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
   glLoadIdentity();
   if (width >= height) {
     // aspect >= 1, set the height from -1 to 1, with larger width
      gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
   } else {
      // aspect < 1, set the width to -1 to 1, with larger height
     gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
   }
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
    glutInit(&argc, argv);          // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE);  // Enable double buffered mode
    glutInitWindowSize(Win_width, Win_height);   // Set the window's initial width & height - non-square
    glutInitWindowPosition(237, 0); // Position the window's initial top-left corner
    glutCreateWindow("3-pawn");  // Create window with the given title
    glutMouseFunc(start);
    glutDisplayFunc(display);       // Register callback handler for window re-paint event
    glutReshapeFunc(reshape);       // Register callback handler for window re-size event
    glutTimerFunc(0, Timer, 0);     // First timer call immediately

    initGL();                       // Our own OpenGL initialization
    glutMainLoop();                 // Enter the infinite event-processing loop
    return EXIT_SUCCESS;
}
