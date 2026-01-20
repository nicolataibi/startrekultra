#define _DEFAULT_SOURCE
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include <unistd.h>

volatile int g_data_dirty = 0;
volatile int g_is_loading = 0;
float angleY = 0.0f;
float angleX = 20.0f;
float zoom = -14.0f;
float autoRotate = 0.15f;
float pulse = 0.0f;

int g_energy = 0, g_shields = 0, g_klingons = 0;
int g_show_axes = 1;
int g_show_grid = 0;
char g_quadrant[128] = "Scanning...";
char g_last_quadrant[128] = "";

typedef struct {
    float x, y, z;
    int type;
} GameObject;

typedef struct {
    float tx, ty, tz;
    float alpha;
} PhaserBeam;

typedef struct {
    float x, y, z;
} Torpedo;

typedef struct {
    float x, y, z;
    int timer;
} Explosion;

typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b;
    int active;
} Particle;

typedef struct {
    float x, y, z;
    int species;
    int timer;
    Particle particles[100];
} Dismantle;

GameObject objects[200];
int objectCount = 0;
Torpedo g_torp = {-100, -100, -100};
Explosion g_boom = {-100, -100, -100, 0};
Dismantle g_dismantle = {-100, -100, -100, 0, 0};
PhaserBeam beams[10];
int beamCount = 0;
#define MAX_TRAIL 100
float enterpriseX = -100, enterpriseY = -100, enterpriseZ = -100;
float ent_trail[MAX_TRAIL][3];
int trail_ptr = 0;
int trail_count = 0;

float stars[1000][3];
void handle_signal(int sig) {
    if (sig == SIGUSR1) g_data_dirty = 1;
}

void initStars() {
    for(int i=0; i<1000; i++) {
        float r = 150.0f + (float)(rand()%100);
        float t = (float)(rand()%360) * 3.14/180;
        float p = (float)(rand()%360) * 3.14/180;
        stars[i][0] = r * sin(p) * cos(t);
        stars[i][1] = r * sin(p) * sin(t);
        stars[i][2] = r * cos(p);
    }
}

void loadGameState() {
    g_is_loading = 1;
    FILE *f = NULL;
    int retries = 0;
    while (f == NULL && retries < 5) {
        f = fopen("/tmp/ultra_map.dat", "r");
        if (f == NULL) usleep(1000);
        retries++;
    }
    if (!f) { g_is_loading = 0; return; }
    
    char line[256];
    GameObject tempObjects[200];
    int tempCount = 0;
    int infoRead = 0;
    float tempEntX = enterpriseX, tempEntY = enterpriseY, tempEntZ = enterpriseZ;
    
    /* Reset transient torpedo position - it will be re-read if active */
    float nextTorpX = -100;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "INFO", 4) == 0) {
            if (sscanf(line, "INFO %d %d %d %127s AXES-%d GRID-%d", &g_energy, &g_shields, &g_klingons, g_quadrant, &g_show_axes, &g_show_grid) == 6) {
                infoRead = 1;
            }
        } else if (strncmp(line, "BEAM", 4) == 0) {
            float tx, ty, tz;
            if (sscanf(line, "BEAM %f %f %f", &tx, &ty, &tz) == 3) {
                int slot = -1;
                for(int i=0; i<10; i++) if(beams[i].alpha <= 0) { slot = i; break; }
                if (slot == -1) slot = rand()%10;
                beams[slot].tx = tx - 5.5f;
                beams[slot].ty = tz - 5.5f; 
                beams[slot].tz = 5.5f - ty;
                beams[slot].alpha = 1.5f; 
                if (beamCount < 10) beamCount++;
            }
        } else if (strncmp(line, "TORP", 4) == 0) {
            float tx, ty, tz;
            if (sscanf(line, "TORP %f %f %f", &tx, &ty, &tz) == 3) {
                nextTorpX = tx - 5.5f;
                g_torp.y = tz - 5.5f;
                g_torp.z = 5.5f - ty;
            }
        } else if (strncmp(line, "BOOM", 4) == 0) {
            float ex, ey, ez;
            if (sscanf(line, "BOOM %f %f %f", &ex, &ey, &ez) == 3) {
                g_boom.x = ex - 5.5f;
                g_boom.y = ez - 5.5f;
                g_boom.z = 5.5f - ey;
                g_boom.timer = 30; 
            }
        } else if (strncmp(line, "DISMANTLE", 9) == 0) {
            float dx, dy, dz;
            int sp;
            if (sscanf(line, "DISMANTLE %f %f %f %d", &dx, &dy, &dz, &sp) == 4) {
                g_dismantle.x = dx - 5.5f;
                g_dismantle.y = dz - 5.5f;
                g_dismantle.z = 5.5f - dy;
                g_dismantle.species = sp;
                g_dismantle.timer = 60;
                for(int i=0; i<100; i++) {
                    g_dismantle.particles[i].x = g_dismantle.x;
                    g_dismantle.particles[i].y = g_dismantle.y;
                    g_dismantle.particles[i].z = g_dismantle.z;
                    g_dismantle.particles[i].vx = ((rand()%100)-50)/500.0f;
                    g_dismantle.particles[i].vy = ((rand()%100)-50)/500.0f;
                    g_dismantle.particles[i].vz = ((rand()%100)-50)/500.0f;
                    g_dismantle.particles[i].r = (rand()%100)/100.0f;
                    g_dismantle.particles[i].g = (rand()%100)/100.0f;
                    g_dismantle.particles[i].b = (rand()%100)/100.0f;
                    g_dismantle.particles[i].active = 1;
                }
            }
        } else {
            float gx, gy, gz;
            int type;
            if (sscanf(line, "%f %f %f %d", &gx, &gy, &gz, &type) == 4) {
                if (isnan(gx) || isnan(gy) || isnan(gz)) continue;
                tempObjects[tempCount].x = gx - 5.5f;
                tempObjects[tempCount].y = gz - 5.5f; 
                tempObjects[tempCount].z = 5.5f - gy;
                tempObjects[tempCount].type = type;
                if (type == 1) { 
                    tempEntX = tempObjects[tempCount].x; 
                    tempEntY = tempObjects[tempCount].y; 
                    tempEntZ = tempObjects[tempCount].z; 
                }
                tempCount++;
                if (tempCount >= 200) break;
            }
        }
    }
    fclose(f);

    /* Only commit changes if we successfully read the data */
    if (infoRead && tempCount > 0) {
        int quadChanged = 0;
        if (g_last_quadrant[0] != '\0' && strcmp(g_quadrant, g_last_quadrant) != 0) {
            quadChanged = 1;
        }
        strcpy(g_last_quadrant, g_quadrant);

        objectCount = tempCount;
        for(int i=0; i<tempCount; i++) objects[i] = tempObjects[i];
        
        enterpriseX = tempEntX;
        enterpriseY = tempEntY;
        enterpriseZ = tempEntZ;
        g_torp.x = nextTorpX;

        if (quadChanged) {
            trail_count = 0;
            trail_ptr = 0;
            /* Pre-fill with new position to prevent ghost segments from previous quadrant */
            for(int j=0; j<MAX_TRAIL; j++) {
                ent_trail[j][0] = enterpriseX;
                ent_trail[j][1] = enterpriseY;
                ent_trail[j][2] = enterpriseZ;
            }
        }

        /* Signal Logic Engine that we are ready for next frame */
        kill(getppid(), SIGUSR2);
    }
    g_is_loading = 0;
}

void drawText3D(float x, float y, float z, const char* text) {
    glRasterPos3f(x, y, z);
    for(int i=0; i<strlen(text); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
}

void drawCompass() {
    glDisable(GL_LIGHTING);
    /* Green Axis: X (Horizontal - Heading 90/270) */
    glColor3f(0, 1, 0); 
    glBegin(GL_LINES); glVertex3f(-5.5,0,0); glVertex3f(5.5,0,0); glEnd();
    drawText3D(5.7, 0, 0, "H: 90"); drawText3D(-7.0, 0, 0, "H: 270");

    /* Blue Axis: Z (Horizontal - Heading 0/180) */
    glColor3f(0, 0, 1); 
    glBegin(GL_LINES); glVertex3f(0,0,-5.5); glVertex3f(0,0,5.5); glEnd();
    drawText3D(0, 0, 5.7, "H: 0"); drawText3D(0, 0, -6.0, "H: 180");

    /* Red Axis: Y (Vertical - Mark +90/-90) */
    glColor3f(1, 0, 0); 
    glBegin(GL_LINES); glVertex3f(0,-5.5,0); glVertex3f(0,5.5,0); glEnd();
    drawText3D(0, 5.7, 0, "MARK +90 (UP)"); drawText3D(0, -6.0, 0, "MARK -90 (DOWN)");
    glEnable(GL_LIGHTING);
}

void drawExplosion() {
    if (g_boom.timer <= 0) return;
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(g_boom.x, g_boom.y, g_boom.z);
    float scale = (31 - g_boom.timer) * 0.03f; /* Max radius around 0.9 (one sector) */
    for (int i=0; i<15; i++) {
        float s = scale * (1.0f - (float)i*0.06f);
        if (s > 0) {
            float r = 1.0f;
            float g = (float)(rand()%100)/100.0f;
            float b = (float)(rand()%50)/100.0f;
            glColor4f(r, g, b, g_boom.timer/30.0f);
            glutSolidSphere(s, 12, 12);
        }
    }
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawDismantle() {
    if (g_dismantle.timer <= 0) return;
    glDisable(GL_LIGHTING);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for(int i=0; i<100; i++) {
        if (g_dismantle.particles[i].active) {
            glColor4f(g_dismantle.particles[i].r, g_dismantle.particles[i].g, g_dismantle.particles[i].b, g_dismantle.timer/60.0f);
            glVertex3f(g_dismantle.particles[i].x, g_dismantle.particles[i].y, g_dismantle.particles[i].z);
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawEnterprise(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(90, 0, 1, 0);
    
    /* Saucer */
    glColor3f(0.8f, 0.8f, 0.9f);
    glPushMatrix(); glScalef(1.0f, 0.15f, 1.0f); glutSolidSphere(0.5, 32, 32); glPopMatrix();
    
    /* Bridge Glow */
    glDisable(GL_LIGHTING); glColor3f(0.0f, 0.5f, 1.0f);
    glPushMatrix(); glTranslatef(0, 0.1f, 0); glutSolidSphere(0.1 + sin(pulse)*0.02, 10, 10); glPopMatrix();
    glEnable(GL_LIGHTING);
    
    /* Engineering Section */
    glColor3f(0.7f, 0.7f, 0.8f);
    glPushMatrix(); glTranslatef(-0.35f, -0.2f, 0); glScalef(1.8f, 0.8f, 0.8f); glutSolidSphere(0.15, 16, 16); glPopMatrix();
    
    /* Nacelles */
    for(int side=-1; side<=1; side+=2) {
        glPushMatrix(); 
        glTranslatef(-0.45f, -0.1f, side * 0.35f); 
        
        /* Nacelle Body */
        glPushMatrix();
        glScalef(4.0f, 0.4f, 0.4f); 
        glColor3f(0.5f, 0.5f, 0.6f); 
        glutSolidSphere(0.1, 12, 12);
        glPopMatrix();

        /* Bussard Collectors (Front Domes) */
        glPushMatrix();
        glTranslatef(0.4f, 0, 0);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.0f, 0.0f);
        glutSolidSphere(0.06, 16, 16);
        glEnable(GL_LIGHTING);
        glPopMatrix();

        glPopMatrix();
    }
    glPopMatrix();
}

void drawRomulan(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.0f, 0.5f, 0.0f); /* Romulan Green */
    /* Double-hull crescent/bird shape */
    glPushMatrix(); glScalef(1.5f, 0.2f, 1.0f); glutSolidSphere(0.4, 16, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0.25f, 0); glScalef(1.5f, 0.2f, 0.8f); glutSolidSphere(0.35, 16, 16); glPopMatrix();
    /* Connecting neck */
    glPushMatrix(); glTranslatef(0.4f, 0.1f, 0); glScalef(1.0f, 0.5f, 0.2f); glutSolidCube(0.3); glPopMatrix();
    /* Beak */
    glColor3f(0.0f, 0.7f, 0.2f);
    glPushMatrix(); glTranslatef(0.7f, 0.1f, 0); glutSolidCone(0.1, 0.3, 8, 8); glPopMatrix();
    glPopMatrix();
}

void drawBorg(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(pulse*5, 1, 1, 1);
    glColor3f(0.3f, 0.3f, 0.3f);
    glutWireCube(0.8);
    glColor3f(0.1f, 0.1f, 0.1f);
    glutSolidCube(0.7);
    /* Internal green lights */
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 1.0f, 0.0f);
    for(int i=0; i<6; i++) {
        glPushMatrix();
        if(i==0) glTranslatef(0.36,0,0); 
        else if(i==1) glTranslatef(-0.36,0,0);
        else if(i==2) glTranslatef(0,0.36,0); 
        else if(i==3) glTranslatef(0,-0.36,0);
        else if(i==4) glTranslatef(0,0,0.36); 
        else if(i==5) glTranslatef(0,0,-0.36);
        glutSolidSphere(0.05, 8, 8);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawCardassian(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.6f, 0.5f, 0.3f); /* Cardassian Tan/Brown */
    /* Galor class delta/fish shape */
    glPushMatrix(); glScalef(2.0f, 0.2f, 1.2f); glutSolidSphere(0.4, 16, 16); glPopMatrix();
    /* Forward spiral structure */
    glColor3f(0.8f, 0.7f, 0.2f);
    glPushMatrix(); glTranslatef(0.5f, 0, 0); glScalef(1.0f, 0.4f, 0.4f); glutSolidSphere(0.2, 12, 12); glPopMatrix();
    glPopMatrix();
}

void drawJemHadar(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.4f, 0.4f, 0.6f); /* Dominion Purple/Grey */
    /* Beetle-like fighter */
    glPushMatrix(); glScalef(1.2f, 0.5f, 1.0f); glutSolidSphere(0.35, 12, 12); glPopMatrix();
    /* Forward mandibles */
    glPushMatrix(); glTranslatef(0.4f, 0, 0.15f); glutSolidCone(0.05, 0.3, 8, 8); glPopMatrix();
    glPushMatrix(); glTranslatef(0.4f, 0, -0.15f); glutSolidCone(0.05, 0.3, 8, 8); glPopMatrix();
    glPopMatrix();
}

void drawTholian(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(pulse*15, 0, 1, 0);
    glColor4f(1.0f, 0.5f, 0.0f, 0.6f); /* Crystalline Orange */
    glDisable(GL_LIGHTING);
    glutWireOctahedron();
    glColor4f(1.0f, 0.2f, 0.0f, 0.4f);
    glutSolidOctahedron();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawGorn(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.3f, 0.4f, 0.1f); /* Gorn Green/Brown */
    /* Blocky, aggressive raider */
    glPushMatrix(); glScalef(1.5f, 0.6f, 0.6f); glutSolidCube(0.4); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.3f, 0, 0); glScalef(0.5f, 1.2f, 1.5f); glutSolidCube(0.3); glPopMatrix();
    glPopMatrix();
}

void drawFerengi(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.7f, 0.3f, 0.1f); /* Ferengi Orange/Brown */
    /* D'Kora Marauder - Crab/Crescent shape */
    glPushMatrix(); glScalef(1.0f, 0.2f, 2.0f); glutSolidSphere(0.4, 16, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(0.3f, 0, 0); glScalef(1.2f, 0.4f, 0.6f); glutSolidSphere(0.3, 12, 12); glPopMatrix();
    glPopMatrix();
}

void drawSpecies8472(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(pulse*10, 1, 0, 1);
    glColor3f(0.8f, 0.8f, 0.2f); /* Organic Yellow */
    /* Bioship - Three pointed organic star */
    for(int i=0; i<3; i++) {
        glPushMatrix();
        glRotatef(i*120, 0, 1, 0);
        glTranslatef(0.3f, 0, 0);
        glScalef(2.0f, 0.3f, 0.3f);
        glutSolidSphere(0.15, 12, 12);
        glPopMatrix();
    }
    glutSolidSphere(0.2, 12, 12);
    glPopMatrix();
}

void drawBreen(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.4f, 0.5f, 0.4f); /* Breen Grey/Green */
    /* Asymmetric Chel Grett */
    glPushMatrix(); glScalef(1.8f, 0.2f, 0.8f); glutSolidCube(0.4); glPopMatrix();
    glPushMatrix(); glTranslatef(0.2f, 0.1f, 0.2f); glScalef(0.5f, 0.5f, 1.2f); glutSolidSphere(0.2, 8, 8); glPopMatrix();
    glPopMatrix();
}

void drawHirogen(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.5f, 0.5f, 0.5f); /* Hirogen Metal */
    /* sleek, jagged hunter ship */
    glPushMatrix(); glScalef(2.5f, 0.15f, 0.4f); glutSolidSphere(0.35, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.4f, 0, 0); glScalef(0.5f, 0.8f, 1.5f); glutSolidCube(0.2); glPopMatrix();
    glPopMatrix();
}

void drawKlingon(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-angleY, 0, 1, 0);
    glColor3f(0.6f, 0.1f, 0.0f);
    glPushMatrix(); glScalef(1.0f, 0.3f, 1.5f); glutSolidSphere(0.3, 16, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(0.4f, 0, 0); glScalef(2.0f, 0.2f, 0.2f); glutSolidSphere(0.15, 8, 8); glPopMatrix();
    glColor3f(0.8f, 0.0f, 0.0f);
    glPushMatrix(); glTranslatef(0.7f, 0, 0); glScalef(1.0f, 0.5f, 1.2f); glutSolidSphere(0.15, 12, 12); glPopMatrix();
    glPopMatrix();
}

void drawStarbase(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(pulse*10, 0, 1, 0);
    glColor3f(0.9f, 0.9f, 0.1f);
    glutWireSphere(0.4, 12, 12);
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix(); glScalef(1.5f, 0.1f, 1.5f); glutSolidCube(0.6); glPopMatrix();
    glPopMatrix();
}

void drawPlanet(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(pulse*5, 0, 1, 0);
    
    /* Planet Body */
    glColor3f(0.2f, 0.6f, 0.3f); /* Earth-like green/blue */
    glutSolidSphere(0.6, 24, 24);
    
    /* Atmosphere/Glow */
    glDisable(GL_LIGHTING);
    glColor4f(0.4f, 0.8f, 1.0f, 0.3f);
    glutSolidSphere(0.65, 24, 24);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
}

void drawBlackHole(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    
    /* Event Horizon - Solid Black */
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);
    glutSolidSphere(0.4, 32, 32);
    
    /* Accretion Disk - Rotating Rings */
    glRotatef(pulse*20, 1, 1, 0);
    for(int i=0; i<5; i++) {
        float r = 0.5f + i*0.15f + sin(pulse*2)*0.05f;
        glColor4f(0.6f - i*0.1f, 0.0f, 0.8f, 0.5f - i*0.1f);
        glutWireTorus(0.02, r, 10, 40);
    }
    
    /* Gravitational Lensing / Glow */
    glColor4f(0.3f, 0.0f, 0.5f, 0.2f);
    glutSolidSphere(1.2, 20, 20);
    
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawPhaserBeams() {
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 10; i++) {
        if (beams[i].alpha > 0) {
            /* Main beam */
            glLineWidth(4.0f);
            glColor4f(1.0f, 0.8f, 0.0f, (beams[i].alpha > 1.0f) ? 1.0f : beams[i].alpha);
            glBegin(GL_LINES);
            glVertex3f(enterpriseX, enterpriseY, enterpriseZ);
            glVertex3f(beams[i].tx, beams[i].ty, beams[i].tz);
            glEnd();
            
            /* Outer glow */
            glLineWidth(10.0f);
            glColor4f(1.0f, 0.3f, 0.0f, (beams[i].alpha > 1.0f ? 0.5f : beams[i].alpha * 0.5f));
            glBegin(GL_LINES);
            glVertex3f(enterpriseX, enterpriseY, enterpriseZ);
            glVertex3f(beams[i].tx, beams[i].ty, beams[i].tz);
            glEnd();
        }
    }
    glEnable(GL_LIGHTING);
    glLineWidth(1.0f);
}

void drawEnterpriseTrail() {
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < trail_count; i++) {
        int idx = (trail_ptr - 1 - i + MAX_TRAIL) % MAX_TRAIL;
        float alpha = (1.0f - (float)i / trail_count) * 0.6f;
        glColor4f(0.4f, 0.7f, 1.0f, alpha);
        glVertex3f(ent_trail[idx][0], ent_trail[idx][1], ent_trail[idx][2]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawGrid() {
    glDisable(GL_LIGHTING);
    glColor4f(0.5f, 0.5f, 0.5f, 0.2f);
    glBegin(GL_LINES);
    for(int i=0; i<=11; i++) {
        float p = -5.5f + (float)i;
        for(int j=0; j<=11; j++) {
            float q = -5.5f + (float)j;
            glVertex3f(p, q, -5.5f); glVertex3f(p, q, 5.5f);
            glVertex3f(p, -5.5f, q); glVertex3f(p, 5.5f, q);
            glVertex3f(-5.5f, p, q); glVertex3f(5.5f, p, q);
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void display() {
    if (g_data_dirty) {
        loadGameState();
        g_data_dirty = 0;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0, zoom);
    glRotatef(angleX, 1, 0, 0); glRotatef(angleY, 0, 1, 0);
    
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    for(int i=0; i<1000; i++) { glColor3f(0.7,0.7,0.7); glVertex3fv(stars[i]); }
    glEnd();
    
    glColor3f(0.2, 0.2, 0.5); glutWireCube(11.0);
    if (g_show_axes) drawCompass();
    if (g_show_grid) drawGrid();
    
    drawPhaserBeams();
    drawEnterpriseTrail();
    drawExplosion();
    drawDismantle();

    /* Render Torpedo */
    if (g_torp.x > -10) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(g_torp.x, g_torp.y, g_torp.z);
        glutSolidSphere(0.12, 10, 10);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
        glutSolidSphere(0.2, 10, 10);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    glEnable(GL_LIGHTING);
    for(int i=0; i<objectCount; i++) {
        switch(objects[i].type) {
            case 1: drawEnterprise(objects[i].x, objects[i].y, objects[i].z); break;
            case 3: drawStarbase(objects[i].x, objects[i].y, objects[i].z); break;
            case 4: /* Stars */
                glDisable(GL_LIGHTING);
                glPushMatrix();
                glTranslatef(objects[i].x, objects[i].y, objects[i].z);
                glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
                glutSolidSphere(0.25, 12, 12);
                for(int layer=1; layer<=3; layer++) {
                    float size = 0.25f + layer * 0.15f + sin(pulse)*0.05f;
                    glColor4f(1.0f, 1.0f - layer*0.1f, 0.0f, 0.4f / layer);
                    glutSolidSphere(size, 16, 16);
                }
                glPopMatrix();
                glEnable(GL_LIGHTING);
                break;
            case 5: drawPlanet(objects[i].x, objects[i].y, objects[i].z); break;
            case 6: drawBlackHole(objects[i].x, objects[i].y, objects[i].z); break;
            case 10: drawKlingon(objects[i].x, objects[i].y, objects[i].z); break;
            case 11: drawRomulan(objects[i].x, objects[i].y, objects[i].z); break;
            case 12: drawBorg(objects[i].x, objects[i].y, objects[i].z); break;
            case 13: drawCardassian(objects[i].x, objects[i].y, objects[i].z); break;
            case 14: drawJemHadar(objects[i].x, objects[i].y, objects[i].z); break;
            case 15: drawTholian(objects[i].x, objects[i].y, objects[i].z); break;
            case 16: drawGorn(objects[i].x, objects[i].y, objects[i].z); break;
            case 17: drawFerengi(objects[i].x, objects[i].y, objects[i].z); break;
            case 18: drawSpecies8472(objects[i].x, objects[i].y, objects[i].z); break;
            case 19: drawBreen(objects[i].x, objects[i].y, objects[i].z); break;
            case 20: drawHirogen(objects[i].x, objects[i].y, objects[i].z); break;
            default: break;
        }
    }
    
    /* UI Overlay */
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 1000, 0, 1000);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glColor3f(0, 1, 1); drawText3D(20, 960, 0, "STAR TREK ULTRA: TRUE 3D NAVIGATION");
    char buf[256]; 
    sprintf(buf, "QUADRANT: %s", g_quadrant); drawText3D(20, 930, 0, buf);
    sprintf(buf, "ENERGY: %d  SHIELDS: %d", g_energy, g_shields); drawText3D(20, 905, 0, buf);
    sprintf(buf, "ENEMIES REMAINING: %d", g_klingons); drawText3D(20, 880, 0, buf);
    
    glColor3f(1, 1, 1);
    drawText3D(20, 50, 0, "Arrows: Rotate | W/S: Zoom | SPACE: Pause | ESC: Exit");
    
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

void timer(int v) { 
    angleY += autoRotate; 
    pulse += 0.1; 
    
    /* Fade out beams */
    for (int i = 0; i < 10; i++) {
        if (beams[i].alpha > 0) beams[i].alpha -= 0.05f;
    }
    
        if (g_boom.timer > 0) g_boom.timer--;
        
        if (g_dismantle.timer > 0) {
            g_dismantle.timer--;
            for(int i=0; i<100; i++) {
                if (g_dismantle.particles[i].active) {
                    g_dismantle.particles[i].x += g_dismantle.particles[i].vx;
                    g_dismantle.particles[i].y += g_dismantle.particles[i].vy;
                    g_dismantle.particles[i].z += g_dismantle.particles[i].vz;
                }
            }
        }
    
                /* Constant trail update (aging) - pushes current pos to eventually clear old trail */
    
                if (enterpriseX > -50 && !g_is_loading) {
    
                    float lastX = ent_trail[(trail_ptr - 1 + MAX_TRAIL) % MAX_TRAIL][0];
    
                    float lastY = ent_trail[(trail_ptr - 1 + MAX_TRAIL) % MAX_TRAIL][1];
    
                    float lastZ = ent_trail[(trail_ptr - 1 + MAX_TRAIL) % MAX_TRAIL][2];
    
                    
    
                    /* Jump detection: if distance is too large, reset trail history */
    
                    float d = sqrtf(powf(enterpriseX - lastX, 2) + powf(enterpriseY - lastY, 2) + powf(enterpriseZ - lastZ, 2));
    
                    if (d > 5.0f && trail_count > 1) {
    
                        trail_count = 0;
    
                        trail_ptr = 0;
    
                    }
    
            
    
                    ent_trail[trail_ptr][0] = enterpriseX;
    
                    ent_trail[trail_ptr][1] = enterpriseY;
    
                    ent_trail[trail_ptr][2] = enterpriseZ;
    
                    trail_ptr = (trail_ptr + 1) % MAX_TRAIL;
    
                    if (trail_count < MAX_TRAIL) trail_count++;
    
                }
    
            
                glutPostRedisplay(); 
        glutTimerFunc(33, timer, 0); 
    }
    void keyboard(unsigned char k, int x, int y) { 
    if(k==27) exit(0); 
    if(k==' ') autoRotate=(autoRotate==0)?0.15:0; 
    if(k=='w' || k=='W') zoom += 0.5f;
    if(k=='s' || k=='S') zoom -= 0.5f;
}
void special(int k, int x, int y) { if(k==GLUT_KEY_UP) angleX-=5; if(k==GLUT_KEY_DOWN) angleX+=5; if(k==GLUT_KEY_LEFT) angleY-=5; if(k==GLUT_KEY_RIGHT) angleY+=5; }

int main(int argc, char** argv) {
    setlocale(LC_ALL, "C");
    signal(SIGUSR1, handle_signal);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("StarTrekUltra - True 3D View");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLfloat lp[] = {10, 10, 10, 1}; glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glLightfv(GL_LIGHT0, GL_POSITION, lp);
    initStars();
        glMatrixMode(GL_PROJECTION); gluPerspective(45, 1.33, 1, 500); glMatrixMode(GL_MODELVIEW);
        glutDisplayFunc(display); glutKeyboardFunc(keyboard); glutSpecialFunc(special); glutTimerFunc(33, timer, 0);
        
        /* Signal parent that initialization is complete */
        kill(getppid(), SIGUSR2);
        
        glutMainLoop();
        return 0;
    }
    