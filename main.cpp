#include <GLUT/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

// Thêm định nghĩa M_PI nếu chưa có (cần cho một số trình biên dịch)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ===============================
// KHAI BÁO TEXTURE SKYBOX
// ===============================
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint skyboxTextures[6]; // 6 mặt của cube map
bool skyboxLoaded = false;

// ===============================
// BIẾN ĐIỀU KHIỂN
// ===============================
float planePosX = -90.0f, planePosY = -0.5f, planePosZ = 0.0f;
float planeYaw = 0.0f;
float planePitch = 0.0f;
float planeRoll = 0.0f;
float speed = 0.5; // Tăng tốc độ một chút
bool turningLeft = false;
bool turningRight = false;
int cameraMode = 1;

// ===============================
// BIẾN ĐỊA HÌNH VÀ CẢNH QUAN
// ===============================
const int TERRAIN_SIZE = 400;
const float TERRAIN_GRID_SCALE = 2.0f;
std::vector<std::vector<float>> terrainHeights;

struct Tree {
    float x, z, y; // Thêm y để lưu độ cao mặt đất
    float scale;
};
std::vector<Tree> trees;

struct House {
    float x, z, y;
    float width, depth, height;
    float r, g, b;
};
std::vector<House> houses;

// =================================================================
// KHAI BÁO CÁC HÀM
// =================================================================
void generateTerrain();
void drawTerrain();
void drawHouse(const House& house);
void drawWater();
float getTerrainHeight(float worldX, float worldZ);
void drawRunway();
void drawTerminal();
void drawControlTower();
void plane();
void drawStraightWing();
void loadSkyboxTexture(); // Hàm load texture skybox
void drawSkybox(); // Hàm vẽ skybox

// =================================================================
// HÀM VẼ CÁNH THẲNG
// =================================================================
void drawStraightWing() {
    const float span = 3.5f;
    const float chord = 1.2f;
    const float thickness = 0.08f;
    const int segments = 20;

    glBegin(GL_QUADS);
    for (int i = 0; i < segments; ++i) {
        float z1 = ((float)i / segments) * span;
        float z2 = ((float)(i + 1) / segments) * span;
        float chord1 = chord * (1.0f - 0.4f * (z1 / span));
        float chord2 = chord * (1.0f - 0.4f * (z2 / span));
        float x_front1 = -chord1 / 2.0f;
        float x_back1 = chord1 / 2.0f;
        float x_front2 = -chord2 / 2.0f;
        float x_back2 = chord2 / 2.0f;

        // Mặt trên
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x_back1, 0.0f, z1);
        glVertex3f(x_front1, 0.0f, z1);
        glVertex3f(x_front2, 0.0f, z2);
        glVertex3f(x_back2, 0.0f, z2);

        // Mặt dưới
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x_back1, -thickness, z1);
        glVertex3f(x_front1, -thickness, z1);
        glVertex3f(x_front2, -thickness, z2);
        glVertex3f(x_back2, -thickness, z2);

        // Cạnh trước
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(x_front1, 0.0f, z1);
        glVertex3f(x_front1, -thickness, z1);
        glVertex3f(x_front2, -thickness, z2);
        glVertex3f(x_front2, 0.0f, z2);

        // Cạnh sau
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(x_back1, 0.0f, z1);
        glVertex3f(x_back1, -thickness, z1);
        glVertex3f(x_back2, -thickness, z2);
        glVertex3f(x_back2, 0.0f, z2);
    }
    glEnd();
}

// =================================================================
// HÀM LOAD TEXTURE SKYBOX
// =================================================================
void loadSkyboxTexture() {
    // Load ảnh cube map gốc
    int width, height, channels;
    unsigned char* data = stbi_load("anyrgb.com.png", &width, &height, &channels, 3);
    
    if (!data) {
        printf("Không thể load texture skybox!\n");
        return;
    }
    
    // Tính toán kích thước của mỗi mặt (ảnh có dạng horizontal cross)
    int faceSize = width / 4; // 4 mặt ngang
    int faceHeight = height / 3; // 3 hàng (top, middle, bottom)
    
    // Tạo texture cho từng mặt
    glGenTextures(6, skyboxTextures);
    
    // Mặt phải (Right) - cột đầu tiên
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[0]);
    unsigned char* rightFace = new unsigned char[faceSize * faceHeight * 3];
    for (int y = 0; y < faceHeight; y++) {
        for (int x = 0; x < faceSize; x++) {
            int srcX = x;
            int srcY = y + faceHeight; // Hàng giữa
            int srcIndex = (srcY * width + srcX) * 3;
            int dstIndex = (y * faceSize + x) * 3;
            rightFace[dstIndex] = data[srcIndex];
            rightFace[dstIndex + 1] = data[srcIndex + 1];
            rightFace[dstIndex + 2] = data[srcIndex + 2];
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSize, faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, rightFace);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] rightFace;
    
    // Mặt trái (Left) - cột cuối
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[1]);
    unsigned char* leftFace = new unsigned char[faceSize * faceHeight * 3];
    for (int y = 0; y < faceHeight; y++) {
        for (int x = 0; x < faceSize; x++) {
            int srcX = x + faceSize * 3; // Cột cuối
            int srcY = y + faceHeight; // Hàng giữa
            int srcIndex = (srcY * width + srcX) * 3;
            int dstIndex = (y * faceSize + x) * 3;
            leftFace[dstIndex] = data[srcIndex];
            leftFace[dstIndex + 1] = data[srcIndex + 1];
            leftFace[dstIndex + 2] = data[srcIndex + 2];
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSize, faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, leftFace);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] leftFace;
    
    // Mặt trên (Up) - hàng đầu
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[2]);
    unsigned char* upFace = new unsigned char[faceSize * faceHeight * 3];
    for (int y = 0; y < faceHeight; y++) {
        for (int x = 0; x < faceSize; x++) {
            int srcX = x + faceSize; // Cột thứ 2
            int srcY = y; // Hàng đầu
            int srcIndex = (srcY * width + srcX) * 3;
            int dstIndex = (y * faceSize + x) * 3;
            upFace[dstIndex] = data[srcIndex];
            upFace[dstIndex + 1] = data[srcIndex + 1];
            upFace[dstIndex + 2] = data[srcIndex + 2];
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSize, faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, upFace);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] upFace;
    
    // Mặt dưới (Down) - hàng cuối
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[3]);
    unsigned char* downFace = new unsigned char[faceSize * faceHeight * 3];
    for (int y = 0; y < faceHeight; y++) {
        for (int x = 0; x < faceSize; x++) {
            int srcX = x + faceSize; // Cột thứ 2
            int srcY = y + faceHeight * 2; // Hàng cuối
            int srcIndex = (srcY * width + srcX) * 3;
            int dstIndex = (y * faceSize + x) * 3;
            downFace[dstIndex] = data[srcIndex];
            downFace[dstIndex + 1] = data[srcIndex + 1];
            downFace[dstIndex + 2] = data[srcIndex + 2];
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSize, faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, downFace);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] downFace;
    
    // Mặt trước (Front) - giữa hàng giữa
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[4]);
    unsigned char* frontFace = new unsigned char[faceSize * faceHeight * 3];
    for (int y = 0; y < faceHeight; y++) {
        for (int x = 0; x < faceSize; x++) {
            int srcX = x + faceSize; // Cột thứ 2
            int srcY = y + faceHeight; // Hàng giữa
            int srcIndex = (srcY * width + srcX) * 3;
            int dstIndex = (y * faceSize + x) * 3;
            frontFace[dstIndex] = data[srcIndex];
            frontFace[dstIndex + 1] = data[srcIndex + 1];
            frontFace[dstIndex + 2] = data[srcIndex + 2];
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSize, faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, frontFace);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] frontFace;
    
    // Mặt sau (Back) - cột thứ 3
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[5]);
    unsigned char* backFace = new unsigned char[faceSize * faceHeight * 3];
    for (int y = 0; y < faceHeight; y++) {
        for (int x = 0; x < faceSize; x++) {
            int srcX = x + faceSize * 2; // Cột thứ 3
            int srcY = y + faceHeight; // Hàng giữa
            int srcIndex = (srcY * width + srcX) * 3;
            int dstIndex = (y * faceSize + x) * 3;
            backFace[dstIndex] = data[srcIndex];
            backFace[dstIndex + 1] = data[srcIndex + 1];
            backFace[dstIndex + 2] = data[srcIndex + 2];
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSize, faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, backFace);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete[] backFace;
    
    stbi_image_free(data);
    skyboxLoaded = true;
    printf("Skybox texture loaded successfully!\n");
}

// =================================================================
// HÀM VẼ SKYBOX
// =================================================================
void drawSkybox() {
    if (!skyboxLoaded) return;
    
    // Lưu trạng thái hiện tại
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL); // Tắt color material để giữ màu original
    glColor3f(1.0f, 1.0f, 1.0f); // Đặt màu trắng để texture hiển thị đúng màu
    
    float size = 200.0f; // Kích thước skybox
    
    // Mặt phải (Right)
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(size, -size, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -size, size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, size, size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(size, size, -size);
    glEnd();
    
    // Mặt trái (Left)
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, size);
    glEnd();
    
    // Mặt trên (Up)
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, size, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, size, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, size, size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, size);
    glEnd();
    
    // Mặt dưới (Down)
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -size, size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, -size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, -size, -size);
    glEnd();
    
    // Mặt trước (Front)
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -size, size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, size, size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, size);
    glEnd();
    
    // Mặt sau (Back)
    glBindTexture(GL_TEXTURE_2D, skyboxTextures[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(size, -size, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(size, size, -size);
    glEnd();
    
    // Khôi phục trạng thái
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL); // Bật lại color material
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

// =================================================================
// HÀM VẼ MÁY BAY
// =================================================================
void plane() {
    glPushMatrix();
    // Thân
    glColor3f(0.3f, 0.6f, 0.3f);
    glPushMatrix();
    glScalef(3, 0.4, 0.5);
    glutSolidSphere(1, 30, 30);
    glPopMatrix();
    // Buồng lái
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(1.7, 0.1, 0);
    glScalef(1.5, 0.7, 0.8);
    glRotatef(40, 0, 1, 0);
    glutSolidSphere(0.45, 30, 30);
    glPopMatrix();
    // Cánh chính
    glColor3f(0.75f, 0.75f, 0.2f);
    glPushMatrix();
    glTranslatef(0.3, 0.0, 0.2);
    drawStraightWing();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.3, 0.0, -0.2);
    glRotatef(180, 0, 1, 0);
    drawStraightWing();
    glPopMatrix();
    // Động cơ
    glColor3f(0.45f, 0.45f, 0.45f);
    float offsetZs[] = {2.0f, 1.0f, -1.0f, -2.0f};
    for (int i = 0; i < 4; ++i) {
        float offsetZ = offsetZs[i];
        glPushMatrix();
        glTranslatef(0.0f, -0.3f, offsetZ);
        glRotatef(90, 0, 1, 0);
        GLUquadric* cyl = gluNewQuadric();
        gluCylinder(cyl, 0.15, 0.15, 0.7, 20, 20);
        gluDeleteQuadric(cyl);
        glPopMatrix();
    }
    // Cánh đuôi ngang
    glColor3f(0.75f, 0.75f, 0.2f);
    glPushMatrix();
    glTranslatef(-2.8, 0, 0);
    glScalef(0.8, 0.5, 0.3);
    glPushMatrix();
    glTranslatef(0.4, 0, 1.5);
    glRotatef(-30, 0, 1, 0);
    glScalef(0.7, 0.1, 3);
    glRotatef(10, 0, 1, 0);
    glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.4, 0, -1.5);
    glRotatef(30, 0, 1, 0);
    glScalef(0.7, 0.1, 3);
    glRotatef(-10, 0, 1, 0);
    glutSolidCube(1);
    glPopMatrix();
    glPopMatrix();
    // Cánh đuôi đứng
    glPushMatrix();
    glTranslatef(-2.7, 0.5, 0);
    glRotatef(45, 0, 0, 1);
    glScalef(0.8, 2, 0.1);
    glRotatef(-20, 0, 0, 1);
    glutSolidCube(0.5);
    glPopMatrix();
    glPopMatrix();
}

// =================================================================
// HÀM VẼ SÂN BAY
// =================================================================
void drawRunway() {
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
    glEnable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);

    GLfloat runway_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat runway_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat runway_shininess[] = { 5.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, runway_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, runway_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, runway_shininess);

    glPushMatrix();
    glTranslatef(0.0f, -0.99f, 0.0f);
    glScalef(200.0f, 0.01f, 10.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    GLfloat stripe_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, stripe_diffuse);
    for (float i = -95.0f; i < 100.0f; i += 10.0f) {
        glPushMatrix();
        glTranslatef(i, -0.98f, 0.0f);
        glScalef(5.0f, 0.01f, 0.2f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    glPopAttrib();
    glEnable(GL_COLOR_MATERIAL);
}

void drawTerminal() {
    glColor3f(0.8f, 0.8f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 25.0f);
    glScalef(80.0f, 6.0f, 10.0f);
    glutSolidCube(1.0);
    glPopMatrix();
    glColor3f(0.6f, 0.8f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 18.0f);
    glScalef(70.0f, 4.0f, 4.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawControlTower() {
    glColor3f(0.8f, 0.8f, 0.7f);
    glPushMatrix();
    glTranslatef(-60.0f, 10.0f, 25.0f);
    glScalef(4.0f, 22.0f, 4.0f);
    glutSolidCube(1.0);
    glPopMatrix();
    glColor3f(0.6f, 0.8f, 0.9f);
    glPushMatrix();
    glTranslatef(-60.0f, 22.0f, 25.0f);
    glScalef(8.0f, 4.0f, 8.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

// =================================================================
// HÀM VẼ CẢNH QUAN
// =================================================================
void drawHouse(const House& house) {
    glPushMatrix();
    glTranslatef(house.x, house.y, house.z);

    // Thân nhà
    glColor3f(house.r, house.g, house.b);
    glPushMatrix();
    glTranslatef(0, house.height / 2.0f, 0);
    glScalef(house.width, house.height, house.depth);
    glutSolidCube(1.0);
    glPopMatrix();

    // Mái nhà
    glColor3f(0.5f, 0.2f, 0.1f);
    float roofHeight = house.height * 0.5f;
    float roofOverhang = 0.5f;
    glBegin(GL_TRIANGLES);
        // Mặt trước
        glVertex3f(-house.width/2 - roofOverhang, house.height, -house.depth/2 - roofOverhang);
        glVertex3f( house.width/2 + roofOverhang, house.height, -house.depth/2 - roofOverhang);
        glVertex3f(0, house.height + roofHeight, -house.depth/2);
        // Mặt sau
        glVertex3f(-house.width/2 - roofOverhang, house.height, house.depth/2 + roofOverhang);
        glVertex3f( house.width/2 + roofOverhang, house.height, house.depth/2 + roofOverhang);
        glVertex3f(0, house.height + roofHeight, house.depth/2);
    glEnd();
    glBegin(GL_QUADS);
        // Mái trái
        glVertex3f(-house.width/2 - roofOverhang, house.height, -house.depth/2 - roofOverhang);
        glVertex3f(0, house.height + roofHeight, -house.depth/2);
        glVertex3f(0, house.height + roofHeight,  house.depth/2);
        glVertex3f(-house.width/2 - roofOverhang, house.height,  house.depth/2 + roofOverhang);
        // Mái phải
        glVertex3f(house.width/2 + roofOverhang, house.height, -house.depth/2 - roofOverhang);
        glVertex3f(0, house.height + roofHeight, -house.depth/2);
        glVertex3f(0, house.height + roofHeight,  house.depth/2);
        glVertex3f(house.width/2 + roofOverhang, house.height,  house.depth/2 + roofOverhang);
    glEnd();
    glPopMatrix();
}

void drawTerrain() {
    glDisable(GL_COLOR_MATERIAL);
    for (int i = 0; i < TERRAIN_SIZE - 1; ++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j < TERRAIN_SIZE; ++j) {
            for (int k = 0; k < 2; ++k) {
                int ci = i + k;
                float x = (ci - TERRAIN_SIZE / 2.0f) * TERRAIN_GRID_SCALE;
                float z = (j - TERRAIN_SIZE / 2.0f) * TERRAIN_GRID_SCALE;
                float y = terrainHeights[ci][j];

                GLfloat mat_diffuse[4];
                if (y < 2.0f) { // Cỏ
                    mat_diffuse[0] = 0.2f; mat_diffuse[1] = 0.6f; mat_diffuse[2] = 0.2f;
                } else if (y < 7.0f) { // Đất
                    mat_diffuse[0] = 0.4f; mat_diffuse[1] = 0.5f; mat_diffuse[2] = 0.2f;
                } else { // Đá
                    mat_diffuse[0] = 0.6f; mat_diffuse[1] = 0.6f; mat_diffuse[2] = 0.6f;
                }
                mat_diffuse[3] = 1.0f;
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
    glEnable(GL_COLOR_MATERIAL);
}

void drawWater() {
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat water_color[] = {0.3f, 0.5f, 0.9f, 0.7f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, water_color);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-200.0f, -0.9f, -200.0f);
    glVertex3f( 200.0f, -0.9f, -200.0f);
    glVertex3f( 200.0f, -0.9f, -150.0f);
    glVertex3f(-200.0f, -0.9f, -150.0f);
    glEnd();

    glPopAttrib();
}


// =================================================================
// CẬP NHẬT VỊ TRÍ MÁY BAY
// =================================================================
void updatePlanePosition() {
    float radYaw = planeYaw * M_PI / 180.0f;
    float radPitch = planePitch * M_PI / 180.0f;
    float dirX = cos(radPitch) * cos(radYaw);
    float dirY = sin(radPitch);
    float dirZ = cos(radPitch) * sin(radYaw);
    planePosX += dirX * speed;
    planePosY += dirY * speed;
    planePosZ += dirZ * speed;
    const float yawSpeed = 0.6f;
    const float rollSpeed = 0.5f;
    const float rollLimit = 20.0f;
    if (turningLeft) {
        planeYaw += yawSpeed;
        if (planeRoll < rollLimit) planeRoll += rollSpeed;
    } else if (turningRight) {
        planeYaw -= yawSpeed;
        if (planeRoll > -rollLimit) planeRoll -= rollSpeed;
    } else {
        if (planeRoll > 0.2f) planeRoll -= rollSpeed;
        else if (planeRoll < -0.2f) planeRoll += rollSpeed;
        else planeRoll = 0.0f;
    }
}

// =================================================================
// VẼ MÀN HÌNH
// =================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float camX, camY, camZ;
    float radYaw = planeYaw * M_PI / 180.0f;
    switch (cameraMode) {
        case 1:
            camX = planePosX - 12.0f * cos(radYaw); camY = planePosY + 4.0f; camZ = planePosZ - 12.0f * sin(radYaw);
            break;
        case 2:
            camX = planePosX + 1.5f * cos(radYaw); camY = planePosY + 0.5f; camZ = planePosZ + 1.5f * sin(radYaw);
            break;
        case 3:
            camX = planePosX - 25.0f * cos(radYaw); camY = planePosY + 8.0f; camZ = planePosZ - 25.0f * sin(radYaw);
            break;
        case 4:
            camX = planePosX; camY = planePosY + 40.0f; camZ = planePosZ + 0.1f;
            break;
    }
    gluLookAt(camX, camY, camZ, planePosX, planePosY, planePosZ, 0, 1, 0);

    // Vẽ skybox trước tiên (bầu trời)
    drawSkybox();

    // Vẽ địa hình và cảnh quan
    drawTerrain();
    drawWater();

    // Vẽ sân bay
    drawRunway();
    drawTerminal();
    drawControlTower();

    // Vẽ cây trên địa hình
    for (const auto& tree : trees) {
        glPushMatrix();
        glTranslatef(tree.x, tree.y, tree.z);
        glScalef(tree.scale, tree.scale, tree.scale);
        // Thân cây
        glColor3f(0.4f, 0.2f, 0.1f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        GLUquadric* trunk = gluNewQuadric();
        gluCylinder(trunk, 0.2, 0.2, 2.5, 8, 3);
        gluDeleteQuadric(trunk);
        glPopMatrix();
        // Tán lá
        glPushMatrix();
        glColor3f(0.0f, 0.6f, 0.0f);
        glTranslatef(0.0f, 2.5f, 0.0f);
        glutSolidSphere(1.2, 10, 10);
        glPopMatrix();
        glPopMatrix();
    }

    // Vẽ nhà cửa
    for (const House& h : houses) {
        drawHouse(h);
    }

    // Vẽ máy bay
    glPushMatrix();
    glTranslatef(planePosX, planePosY, planePosZ);
    glRotatef(-planeYaw, 0, 1, 0);
    glRotatef(planePitch, 0, 0, 1);
    glRotatef(planeRoll, 1, 0, 0);
    plane();
    glPopMatrix();

    glutSwapBuffers();
}

// =================================================================
// CÁC HÀM TIỆN ÍCH
// =================================================================
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void idle() {
    updatePlanePosition();
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  turningLeft = true; break;
        case GLUT_KEY_RIGHT: turningRight = true; break;
        case GLUT_KEY_UP:    planePitch += 1.0f; if (planePitch > 45) planePitch = 45; break;
        case GLUT_KEY_DOWN:  planePitch -= 1.0f; if (planePitch < -45) planePitch = -45; break;
    }
}

void specialKeyUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  turningLeft = false;
    if (key == GLUT_KEY_RIGHT) turningRight = false;
}

void keyboard(unsigned char key, int x, int y) {
    if (key >= '1' && key <= '4') cameraMode = key - '0';
}

float getTerrainHeight(float worldX, float worldZ) {
    float gridX = worldX / TERRAIN_GRID_SCALE + TERRAIN_SIZE / 2.0f;
    float gridZ = worldZ / TERRAIN_GRID_SCALE + TERRAIN_SIZE / 2.0f;
    int x0 = static_cast<int>(gridX);
    int z0 = static_cast<int>(gridZ);
    if (x0 < 0 || x0 >= TERRAIN_SIZE - 1 || z0 < 0 || z0 >= TERRAIN_SIZE - 1) return -1.0f;

    float tx = gridX - x0;
    float tz = gridZ - z0;
    float h00 = terrainHeights[x0][z0];
    float h10 = terrainHeights[x0 + 1][z0];
    float h01 = terrainHeights[x0][z0 + 1];
    float h11 = terrainHeights[x0 + 1][z0 + 1];

    float h_interp_z1 = (1 - tx) * h00 + tx * h10;
    float h_interp_z2 = (1 - tx) * h01 + tx * h11;
    return (1 - tz) * h_interp_z1 + tz * h_interp_z2;
}

void generateTerrain() {
    terrainHeights.resize(TERRAIN_SIZE, std::vector<float>(TERRAIN_SIZE));
    for (int i = 0; i < TERRAIN_SIZE; ++i) {
        for (int j = 0; j < TERRAIN_SIZE; ++j) {
            float x = (float)i - TERRAIN_SIZE / 2.0f;
            float z = (float)j - TERRAIN_SIZE / 2.0f;
            float height = sin(x * 0.05f) * cos(z * 0.03f) * 8.0f;
            height += sin(x * 0.1f) * 3.0f - 1.0f;

            float worldX = x * TERRAIN_GRID_SCALE;
            float worldZ = z * TERRAIN_GRID_SCALE;
            if (worldX > -110 && worldX < 110 && worldZ > -50 && worldZ < 50) {
                 height = -1.0f;
            }
            terrainHeights[i][j] = height;
        }
    }
}

void init() {
    glClearColor(0.7f, 0.85f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_position[] = { 10.0f, 100.0f, 10.0f, 0.0f };
    GLfloat light_ambient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light_diffuse[]  = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Load skybox texture
    loadSkyboxTexture();

    srand(time(0));
    generateTerrain();

    for (int i = 0; i < 800; ++i) {
        float x = (float)(rand() % 400 - 200);
        float z = (float)(rand() % 400 - 200);
        if (z > -50 && z < 50 && x > -110 && x < 110) continue;
        if (z < -140) continue;
        float y = getTerrainHeight(x, z);
        if (y < -0.8f) continue;
        float scale = (float)(rand() % 50) / 100.0f + 0.8f;
        trees.push_back({x, z, y, scale});
    }

    for (int i = 0; i < 100; ++i) {
        float x = (float)(rand() % 380 - 190);
        float z = (float)(rand() % 300 - 140);
        if (z > -50 && z < 50 && x > -110 && x < 110) continue;
        if (z < -140) continue;
        float y = getTerrainHeight(x, z);
        if (y < -0.8f) continue;
        House newHouse;
        newHouse.x = x; newHouse.y = y; newHouse.z = z;
        newHouse.width = (float)(rand() % 4) + 4.0f;
        newHouse.depth = (float)(rand() % 4) + 4.0f;
        newHouse.height = (float)(rand() % 5) + 5.0f;
        newHouse.r = (float)(rand() % 100) / 200.0f + 0.5f;
        newHouse.g = (float)(rand() % 100) / 200.0f + 0.5f;
        newHouse.b = (float)(rand() % 100) / 200.0f + 0.5f;
        houses.push_back(newHouse);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Mo Phong May Bay - San Bay (Da Cai Tien)");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeyUp);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
