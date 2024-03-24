/*
 * (C) X-prog 2003
 *
 * Author: Rui Jorge Pereira Gonçalves
 *
 * OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
 */

/*
 *  nbody.c
 *  Este programa mostra a simulação de duas galaxia a interagirem
 *  com as respectivas forças de gravidade
 */

#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

/*--------------------------------simulation vars-------------------------*/
/* some constants */

#define   pi          M_PI
#define   MASS_UNIT   (double)5.0
#define   SOFT        (double)2.0
#define   ONEDOT5     (double)1.5
#define   NUCLEUS     (double)5.0
#define   DOT5        (double)0.5
#define   DOT9        (double)0.9
#define   DEG_TO_RAD  (double)(pi/180.0)
#define   RAD_TO_DEG  (double)(180.0/pi)
#define   DEG_IN_2PI  (double)360.0
#define   SQR(X)      ((X)*(X))

/* simulation parameters */

/* number of time steps */

int     time_steps;

/* bodies are assumed to have 0-mass and their movement
   depends solely on the galaxy's gravitational center */

typedef struct{
    double  pos[3];
    double  vel[3];
}body;

typedef struct{
    double  radius;     /* radius of galaxy disk */
    int     nbodies;    /* number of bodies in galaxy */
    double  mass;       /* galaxy mass */
    double  pos[3];     /* position of galaxy center */
    double  vel[3];     /* velocity of galaxy center */
    double  theta;      /* initial angle between disk and x-axis */
    double  phi;        /* initial angle between disk and xy plane */
    body   *bodies;     /* galaxy bodies */
}galaxy;

static galaxy  target;
static galaxy  intruder;

/* some interfaces */
// abaixo do main
static void  get_experiment_parameters();
static void  initial_conditions();


/*----------------------------------  Visual vars-------------------------- */
static int year = 0, day = 0,i=0,flag_pause=0,xant,yant,Mouse,view_mode=0;
static int flag_balls=0,flag_ref=0;
static double  Vang=0.001,Hang=0.0,Zoom=100.0;


void init(void)
{
  GLfloat mat[3] = {0.8, 0.8, 0.8};
  GLfloat position[] = { 10., 10., 10.0, 0.0 };

  glShadeModel (GL_SMOOTH);   //GL_FLAT +rapido

  glClearColor (0.0, 0.0, 0.1, 0.0);
  glEnable(GL_DEPTH_TEST);


  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  {
    glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
    mat[0] = 0.61424; mat[1] = 0.64136; mat[2] = 6.04136;
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
    mat[0] = 0.727811; mat[1] = 0.626959; mat[2] = 0.626959;
    glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
    glMaterialf (GL_FRONT, GL_SHININESS, 0.6*128.0);
  }
  glPointSize(1.1);
  glLightfv(GL_LIGHT0, GL_POSITION, position);   //posicionar luz
}


// Esta função desenha o texto passado como argumento na posição x,y com a font dada
// Função "roubada" de http://www.tjhsst.edu/~dhyatt/superap/opengl.html
void DisplayText(double x, double y, char *string, void *font) {

  int len, i;

  glRasterPos2f(x, y);               // Locate Raster Position in 3-space
  len = (int) strlen(string);        // Find length of string
  for (i = 0; i < len; i++) {        // Loop through plotting all characters in font style
    glutBitmapCharacter(font, string[i]);
  }
}



static void renderSphere (GLfloat x, GLfloat y, GLfloat z, GLfloat Raio)
{
   glPushMatrix();
   glTranslatef (x, y, z);
   glutSolidSphere(Raio, 8, 8);
   glPopMatrix();
}

static void renderSphere2 (GLfloat x, GLfloat y, GLfloat z, GLfloat Raio)
{
   glPushMatrix();
   glTranslatef (x, y, z);
   glutSolidSphere(Raio, 3, 3);
   glPopMatrix();
}

double pos(double x)
{
  if (x<0)
    return x*(-1);
  else
    return x;
}

void display(void)
{
  //simulation
  int     k, tnbodies = target.nbodies + intruder.nbodies;
  double  x, y, z;
  double x0, y0, z0;
  double x1, y1, z1;
  double xc, yc, zc;
  double  dradius = NUCLEUS;
  GLfloat vaux=0;

  // View engine
/*  double xeye=Zoom*sin(Vang)*cos(Hang);
  double yeye=Zoom*sin(Vang)*sin(Hang);
  double zeye=Zoom*cos(Vang);*/
  char saux[20];
  GLfloat mat[3] = {0.8, 0.8, 0.8};
  glMaterialfv (GL_FRONT, GL_AMBIENT, mat);

//  int aux=(int)(Vang/ M_PI);

 glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  if (view_mode==0)
    {
      glScaled( 15/Zoom,15/Zoom,15/Zoom);
      glOrtho(-10.0, 10.0, -10.0, 10.0,0.1, 10000.0);
    }
  else
    {
      glScaled( 1.0,1.0,1.0);
      glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 10000.0);
    }
  glMatrixMode(GL_MODELVIEW);


/*  glLoadIdentity();
  if (sin(Vang)>0.)
    gluLookAt (xeye, yeye, zeye, //onde
	       0.0, 0.0, 0.0, //para
	       0.0, 0.0, 1.0);//orientação
  else
    gluLookAt (xeye, yeye, zeye, //onde
	       0.0, 0.0, 0.0, //para
	       0.0, 0.0, -1.0);//orientação*/


   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   //   glColor3f (1.0, 1.0, 1.0);


   if (flag_ref==0)  //por referencial
     {
       glDisable(GL_LIGHTING);
       glBegin(GL_LINES);
       glColor3f (0.5,0.5,0.5);
       glVertex3f(100.0,0.0,0.0);
       glVertex3f(0,0,0);
       glVertex3f(0.0,100.0,0.0);
       glVertex3f(0,0,0);
       glVertex3f(0.0,0.0,100.0);
       glVertex3f(0,0,0);
       glEnd();
     }
   glEnable(GL_LIGHTING);

   body   *body_ptr = target.bodies;
   /* first, the target galaxy */

  /* the center */
  xc = ((intruder.pos[0])*intruder.mass + (target.pos[0])*target.mass)
       / (target.mass+intruder.mass);
  yc = ((intruder.pos[1])*intruder.mass + (target.pos[1])*target.mass)
       / (target.mass+intruder.mass);
  zc = ((intruder.pos[2])*intruder.mass + (target.pos[2])*target.mass )
       /(target.mass+intruder.mass);

  x  = (target.pos[0]) - xc;
  y  = (target.pos[1]) - yc;
  z  = (target.pos[2]) - zc;

  mat[0] = 0.0;mat[1]=1.0;mat[2]=0.0;
  glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
  glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
  glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
  renderSphere(x, y, z,1.0);

  /* the bodies */
  if (flag_balls==0)
    {

      glDisable(GL_LIGHTING);
      glBegin(GL_POINTS);
      for( k = 0; k < target.nbodies ; k++ )
	{
	  x = (body_ptr[k].pos[0]) - xc;
	  y = (body_ptr[k].pos[1]) - yc;
	  z = (body_ptr[k].pos[2]) - zc;
	  // ideia da norma da vel
	  vaux=(GLfloat)((pos(body_ptr[k].vel[0])+pos(body_ptr[k].vel[1])+pos(body_ptr[k].vel[2]))*1.1);
	  glColor3f(0.0,vaux,0.0);
	  glVertex3f(x,y,z);

	}
      glEnd();
      glEnable(GL_LIGHTING);
    }
  else
    {
      for( k = 0; k < target.nbodies ; k++ )
	{
	  x = (body_ptr[k].pos[0]) - xc;
	  y = (body_ptr[k].pos[1]) - yc;
	  z = (body_ptr[k].pos[2]) - zc;
	  // ideia da norma da vel
	  vaux=(GLfloat)((pos(body_ptr[k].vel[0])+pos(body_ptr[k].vel[1])+pos(body_ptr[k].vel[2]))*1.1);
	  mat[0] = 0.0;mat[1]=vaux;mat[2]=0.0;
	  glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
	  glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
	  glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
	  renderSphere2(x, y, z,0.2);
	}
    }
  /* second, the intruder galaxy */

  /* the center */

  x = (intruder.pos[0]) - xc;
  y = (intruder.pos[1]) - yc;
  z = (intruder.pos[2]) - zc;
  mat[0] = 1.0;mat[1]=0.0;mat[2]=0.0;
  glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
  glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
  glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
  renderSphere(x, y, z,1.0); //bola centro



  /* the bodies */
  if(flag_balls==0)
    {

      glDisable(GL_LIGHTING);
      glBegin(GL_POINTS);
      for( k = target.nbodies; k < intruder.nbodies + target.nbodies ; k++ )
	{
	  x = (body_ptr[k].pos[0]) - xc;
	  y = (body_ptr[k].pos[1]) - yc;
	  z = (body_ptr[k].pos[2]) - zc;

	  vaux=(GLfloat)((pos(body_ptr[k].vel[0])+pos(body_ptr[k].vel[1])+pos(body_ptr[k].vel[2]))*1.1);
	  if(flag_balls==0)
	    {
	      glColor3f(vaux,0.0,0.0);
	      glVertex3f(x,y,z);
	    }
	}
      glEnd();
      glEnable(GL_LIGHTING);
    }
  else
    {
      for( k = target.nbodies; k < intruder.nbodies + target.nbodies ; k++ )
	{
	  x = (body_ptr[k].pos[0]) - xc;
	  y = (body_ptr[k].pos[1]) - yc;
	  z = (body_ptr[k].pos[2]) - zc;

	  vaux=(GLfloat)((pos(body_ptr[k].vel[0])+pos(body_ptr[k].vel[1])+pos(body_ptr[k].vel[2]))*1.1);
	  mat[0] = vaux;mat[1]=0.0;mat[2]=0.0;
	  glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
	  glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
	  glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
	  renderSphere2(x, y, z,0.2);
	}
    }

  //------------texto
  glPushMatrix();

  glDisable(GL_LIGHTING);
  glColor3f (1.0, 1.0, 1.0);
  glLoadIdentity();

  glMatrixMode (GL_PROJECTION);
//  glPushMatrix();
  glLoadIdentity ();
  glOrtho(-10.0, 10.0, -10.0, 10.0,0.5, 10000.0);
  glMatrixMode(GL_MODELVIEW);

  gluLookAt (0.0, 0.0,10.0, //onde
	       0.0, 0.0, 0.0, //para
	       0.0, 1.0, 0.0);//orientação

  glTranslatef (-9.5, -9.0, 0.0);
  if (flag_pause==0)
    DisplayText(0.0, 0.0, "[P]ause:On",GLUT_BITMAP_HELVETICA_10);
  else
    DisplayText(0.0, 0.0, "[P]ause:Off", GLUT_BITMAP_HELVETICA_10);


  glTranslatef (2.5, 0.0, 0.0);
  if (view_mode==0)
    DisplayText(0.0, 0.0, "[V]iew:Ortho",GLUT_BITMAP_HELVETICA_10);
  else
    DisplayText(0.0, 0.0, "[V]iew:Frustum", GLUT_BITMAP_HELVETICA_10);

  glTranslatef (3.0, 0.0, 0.0);
  if (flag_balls==0)
    DisplayText(0.0, 0.0, "[B]alls:Off",GLUT_BITMAP_HELVETICA_10);
  else
    DisplayText(0.0, 0.0, "[B]alls:On", GLUT_BITMAP_HELVETICA_10);


 glTranslatef (2.5, 0.0, 0.0);
  if (flag_ref==0)
    DisplayText(0.0, 0.0, "[R]ef:On",GLUT_BITMAP_HELVETICA_10);
  else
    DisplayText(0.0, 0.0, "[R]ef:Off", GLUT_BITMAP_HELVETICA_10);

  glTranslatef (2.5, 0.0, 0.0);
 // sprintf(saux,"Eye coord:%f,%f,%f",xeye,yeye,zeye);
 // DisplayText(0.0, 0.0, saux,GLUT_BITMAP_HELVETICA_10);
  glPopMatrix();

  //glMatrixMode (GL_PROJECTION);
  //glPopMatrix();
  //glMatrixMode(GL_MODELVIEW);

  //glEnable(GL_LIGHTING);
  glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();

   //glOrtho(-10.0, 10.0, -10.0, 10.0,0.5, 10000.0);
      glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 10000.0);
   // gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);

   glMatrixMode(GL_MODELVIEW);
   //glLoadIdentity();
   //gluLookAt (3.0, 0.0, 3.0, 0.0, 0.0, 0., 0.0, 1.0, 0.0);
}

void mouse(int button, int state, int x, int y)
{

  if (state== GLUT_DOWN)   //presiona um button para
     {
       flag_pause=0;
       Mouse=button;
       /* Mouse buttons. */
     }
  else
    {
      Mouse=-1;
      flag_pause=1;
    }

  xant=x;
  yant=y;
 // printf("x:%d  y:%d\n",x,y);


}


void motion(int x, int y)
{


  switch(Mouse)
    {
    case GLUT_LEFT_BUTTON:
      if(y>yant)
	Vang-=((double)(y-yant))/20;
      else
	if (y<yant) Vang-=((double)(y-yant))/20;
      if(x>xant)
	Hang+=((double)(x-xant))/20;
      else
	if (x<xant) Hang+=((double)(x-xant))/20;
      break;
    case GLUT_RIGHT_BUTTON:
      if(y>yant)
	Zoom-=((double)(y-yant))/20;
      else
	if (y<yant) Zoom-=((double)(y-yant))/20;
      break;
    }

  double xeye=Zoom*sin(Vang)*cos(Hang);
  double yeye=Zoom*sin(Vang)*sin(Hang);
  double zeye=Zoom*cos(Vang);

  //printf("V:%f  H:%f\n",Vang,Hang);
  xant=x;
  yant=y;

  glLoadIdentity();
  if (sin(Vang)>0.)
    gluLookAt (xeye, yeye, zeye, //onde
	       0.0, 0.0, 0.0, //para
	       0.0, 0.0, 1.0);//orientação
  else
    gluLookAt (xeye, yeye, zeye, //onde
	       0.0, 0.0, 0.0, //para
	       0.0, 0.0, -1.0);//orientação


}


void keyboard (unsigned char key, int x, int y)
{
  switch (key) {
  case 'd':
    day = (day + 10) % 360;
    glutPostRedisplay();
    break;
  case 'r':
    if (flag_ref==0)
      flag_ref =1;
    else
      flag_ref =0;    break;
  case 'b':
    if (flag_balls==0)
      flag_balls =1;
    else
      flag_balls =0;
    break;
  case 'v':
    if (view_mode==0)
      view_mode=1;
    else
      view_mode=0;
    break;
  case 'p':
    if (flag_pause==0)
      flag_pause=1;
    else
      flag_pause=0;
    break;
  case 27:
    exit(0);
    break;
  default:
    break;
  }


}

void rui(void)
{
 int       k, tnbodies;
  body    *bodiesp = target.bodies;
  double   gcforce, force[2], accel[3];

  tnbodies = (target.nbodies) + (intruder.nbodies);

  if(flag_pause)
   {
      for( k = 0 ; k < tnbodies ; k++ )  /* for all bodies */
        {
          force[0] = (target.mass) / pow(
                     SQR(bodiesp[k].pos[0] - target.pos[0]) +
                     SQR(bodiesp[k].pos[1] - target.pos[1]) +
                     SQR(bodiesp[k].pos[2] - target.pos[2]) +
                     SOFT, ONEDOT5 );

          force[1] = (intruder.mass) / pow(
                     SQR(bodiesp[k].pos[0] - intruder.pos[0]) +
                     SQR(bodiesp[k].pos[1] - intruder.pos[1]) +
                     SQR(bodiesp[k].pos[2] - intruder.pos[2]) +
                     SOFT, ONEDOT5 );

          /* total acceleration on body */

          accel[0] = force[0]*(target.pos[0]-bodiesp[k].pos[0]) +
                     force[1]*(intruder.pos[0]-bodiesp[k].pos[0]);

          accel[1] = force[0]*(target.pos[1]-bodiesp[k].pos[1]) +
                     force[1]*(intruder.pos[1]-bodiesp[k].pos[1]);

          accel[2] = force[0]*(target.pos[2]-bodiesp[k].pos[2]) +
                     force[1]*(intruder.pos[2]-bodiesp[k].pos[2]);

          /* calculate new position using */
          /* Leap-Frog, time centered algorithm */

          bodiesp[k].vel[0] += accel[0];
          bodiesp[k].vel[1] += accel[1];
          bodiesp[k].vel[2] += accel[2];

          bodiesp[k].pos[0] += bodiesp[k].vel[0];
          bodiesp[k].pos[1] += bodiesp[k].vel[1];
          bodiesp[k].pos[2] += bodiesp[k].vel[2];
	}

      /* update galactic center coordinates */

      gcforce = pow( SQR(target.pos[0] - intruder.pos[0]) +
                     SQR(target.pos[1] - intruder.pos[1]) +
                     SQR(target.pos[2] - intruder.pos[2]) +
                     SOFT, ONEDOT5 );

      accel[0] = (intruder.pos[0] - target.pos[0]) / gcforce;
      accel[1] = (intruder.pos[1] - target.pos[1]) / gcforce;
      accel[2] = (intruder.pos[2] - target.pos[2]) / gcforce;

      target.vel[0] += (intruder.mass) * accel[0];
      target.vel[1] += (intruder.mass) * accel[1];
      target.vel[2] += (intruder.mass) * accel[2];

      intruder.vel[0] -= (target.mass) * accel[0];
      intruder.vel[1] -= (target.mass) * accel[1];
      intruder.vel[2] -= (target.mass) * accel[2];

      target.pos[0] += target.vel[0];
      target.pos[1] += target.vel[1];
      target.pos[2] += target.vel[2];

      intruder.pos[0] += intruder.vel[0];
      intruder.pos[1] += intruder.vel[1];
      intruder.pos[2] += intruder.vel[2];

//      display_bodies( /*target, intruder*/ );  // glutPostRedisplay();
    }


  //    i++;
  //printf("i:%d\n",i);
  glutPostRedisplay();
}

int gl(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (500, 500);
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("Galaxy  (c)X-prog 2003");
   init ();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(rui);       //função que está sempre a correr rui()
   glutMainLoop();
   return 0;
}
/***********************************************************************
 *                MAIN()                                               *
 *                                                                     *
 ***********************************************************************/
int main(int argc, char** argv)
{
  //Simulação
  get_experiment_parameters();
  initial_conditions();

  // View engine
  gl(argc,argv);
  return 0;
}



void  get_experiment_parameters()
{
  int     tnbodies;
  char    line[128];

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%lf,%lf", &(target.radius), &(intruder.radius) );
  printf("\nradius of target and intruder galaxies -> (%lf,%lf)",
          target.radius, intruder.radius );

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%d,%d", &(target.nbodies), &(intruder.nbodies) );
  printf("\n# of stars in target and in intruder -> (%d,%d): ",
          target.nbodies, intruder.nbodies );

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%lf", &(intruder.mass) );
  printf("\nmass fraction of intruder relative to target -> %lf", intruder.mass );

  /* calculate individual masses */

  target.mass    = MASS_UNIT;
  intruder.mass *= target.mass;

  /* initial positions */

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%lf,%lf,%lf", &(intruder.pos[0]),
          &(intruder.pos[1]), &(intruder.pos[2]) );
  printf("\ninitial position of intruder -> (%lf,%lf,%lf)",
          intruder.pos[0], intruder.pos[1], intruder.pos[2] );

  target.pos[0] = target.pos[1] = target.pos[2] = 0.0;

  /* initial velocities */

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%lf,%lf,%lf", &(intruder.vel[0]),
          &(intruder.vel[1]), &(intruder.vel[2]) );
  printf("\ninitial velocity of intruder -> (%lf,%lf,%lf)",
          intruder.vel[0], intruder.vel[1], intruder.vel[2] );

  target.vel[0] = target.vel[1] = target.vel[2] = 0.0;

  /* initial orientation of intruder relative to target */

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%lf,%lf", &(intruder.theta), &(intruder.phi) );
  printf("\ninitial orientation of intruder -> (%lf,%lf)",
          intruder.theta, intruder.phi );

  target.theta = target.phi = 0.0;

  /* create the body arrays */

  tnbodies = target.nbodies + intruder.nbodies;
  target.bodies   = (body *)malloc( tnbodies * sizeof(body) );
  intruder.bodies = target.bodies + target.nbodies;

  /* and finally ... get the number of time steps */

  (void)fgets( line, sizeof(line), stdin );
  sscanf( line, "%d", &time_steps );
  printf("\n# of time steps for this experiment -> %d\n\n", time_steps );
}



void initial_conditions()
{
  int     i;
  double  r, v, th, t, t1;
  double  itheta_rad, iphi_rad;
  double  cosphi, sinphi, costheta, sintheta;

  /* initialize target */

  srand48(getpid());
  for( i = 0 ; i < target.nbodies ; i++ )
    {
       r = NUCLEUS + drand48()*target.radius;
       v = sqrt( target.mass / r );
       th = (DOT5 * v / r) * RAD_TO_DEG;
       t = drand48() * DEG_IN_2PI;
       t1 = DEG_TO_RAD * (t - th);

       target.bodies[i].pos[0] = r * cos( t * DEG_TO_RAD );
       target.bodies[i].pos[1] = r * sin( t * DEG_TO_RAD );
       target.bodies[i].pos[2] = 0;
       target.bodies[i].vel[0] = -v * sin( t1 );
       target.bodies[i].vel[1] =  v * cos( t1 );
       target.bodies[i].vel[2] =  0;
    }

  /* initialize intruder */

  itheta_rad = intruder.theta * DEG_TO_RAD;
  iphi_rad   = intruder.phi   * DEG_TO_RAD;
  cosphi = cos( iphi_rad );
  sinphi = sin( iphi_rad );
  costheta = cos( itheta_rad );
  sintheta = sin( itheta_rad );

  for( i = 0 ; i < intruder.nbodies ; i++ )
    {
      double x, y, z, vx, vy, vz, aux;

      r = NUCLEUS + drand48() * intruder.radius;
      v = sqrt( intruder.mass / r );
      th = (DOT5 * v / r) * RAD_TO_DEG;
      t = drand48() * DEG_IN_2PI;
      t1 = DEG_TO_RAD * (t - th);

      x = r * cos( t * DEG_TO_RAD );
      y = r * sin( t * DEG_TO_RAD );
      z = 0;
      vx = -v * sin( t1 );
      vy =  v * cos( t1 );
      vz =  0;

      /* phi - rotation */

      aux = x * cosphi - z * sinphi;
      z = x * sinphi + z * cosphi;
      x = aux;
      aux = vx * cosphi - vz * sinphi;
      vz = vx * sinphi + vz * cosphi;
      vx = aux;
      /* y = y & vy = vy */

      /* theta - rotation */

      aux = x * costheta - y * sintheta;
      y = x * sintheta + y * costheta;
      x = aux;
      aux = vx * costheta - vy * sintheta;
      vy = vx * sintheta + vy * costheta;
      vx = aux;
      /* z = z & vz = vz */


      /* and finally ... */
      intruder.bodies[i].pos[0] = x + intruder.pos[0];
      intruder.bodies[i].pos[1] = y + intruder.pos[1];
      intruder.bodies[i].pos[2] = z + intruder.pos[2];
      intruder.bodies[i].vel[0] = vx + intruder.vel[0];
      intruder.bodies[i].vel[1] = vy + intruder.vel[1];
      intruder.bodies[i].vel[2] = vz + intruder.vel[2];

       i++;
    }

}



