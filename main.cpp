#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

// ===============================
// BIẾN ĐIỀU KHIỂN
// ===============================

// THAY ĐỔI: Cập nhật vị trí xuất phát của máy bay để nằm trên đường băng
float planePosX = -90.0f, planePosY = -0.5f, planePosZ = 0.0f;
float planeYaw = 0.0f; // Hướng ban đầu dọc theo đường băng
float planePitch = 0.0f;
float planeRoll = 0.0f;
float speed = 0.03f;

// Trạng thái phím
bool turningLeft = false;
bool turningRight = false;

// Camera góc nhìn
int cameraMode = 1;

// Struct mây và tòa nhà
struct Cloud {
    float x, y, z;
};
std::vector<Cloud> clouds;

struct Building {
    float x, z;
    float height;
};
std::vector<Building> buildings;

// Struct để quản lý cây cối
struct Tree {
    float x, z;
    float scale;
};
std::vector<Tree> trees;


// =================================================================
// KHAI BÁO CÁC HÀM VẼ SÂN BAY
// =================================================================
void drawRunway();
void drawTerminal();
void drawControlTower();


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
// HÀM VẼ ĐƯỜNG BĂNG (PHIÊN BẢN SỬA LỖI CUỐI CÙNG)
// =================================================================
void drawRunway() {
    // Lưu lại các trạng thái của OpenGL sẽ bị thay đổi
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);

    // Bật chế độ tự động chuẩn hóa vector pháp tuyến
    glEnable(GL_NORMALIZE);

    // Tạm thời tắt GL_COLOR_MATERIAL để glMaterialfv có toàn quyền
    glDisable(GL_COLOR_MATERIAL);

    // --- BẮT ĐẦU ĐỊNH NGHĨA VẬT LIỆU CHO ĐƯỜNG BĂNG ---

    // 1. Vật liệu cho phần đường băng màu xám (nhựa đường)
    GLfloat runway_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };  // Màu khuếch tán (màu chính) là xám
    GLfloat runway_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // Hầu như không phản xạ gương
    GLfloat runway_shininess[] = { 5.0f };                  // Độ bóng rất thấp

    // Áp dụng vật liệu màu xám
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, runway_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, runway_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, runway_shininess);

    // Vẽ đường băng chính
    glPushMatrix();
    glTranslatef(0.0f, -0.99f, 0.0f);
    glScalef(200.0f, 0.01f, 10.0f);
    glutSolidCube(1.0);
    glPopMatrix();


    // 2. Vật liệu cho vạch kẻ màu trắng
    GLfloat stripe_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Màu khuếch tán là trắng

    // Áp dụng vật liệu màu trắng (vẫn dùng độ bóng thấp của đường băng)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, stripe_diffuse);

    // Vẽ các vạch kẻ
    for (float i = -95.0f; i < 100.0f; i += 10.0f) {
        glPushMatrix();
        glTranslatef(i, -0.98f, 0.0f);
        glScalef(5.0f, 0.01f, 0.2f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // --- KẾT THÚC ĐỊNH NGHĨA VẬT LIỆU ---

    // Khôi phục lại toàn bộ các trạng thái đã lưu
    glPopAttrib();
}

// Hàm vẽ nhà ga sân bay
void drawTerminal() {
    // Tòa nhà chính
    glColor3f(0.8f, 0.8f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 25.0f);
    glScalef(80.0f, 6.0f, 10.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Tháp kính phía trước
    glColor3f(0.6f, 0.8f, 0.9f); // Màu xanh kính
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 18.0f);
    glScalef(70.0f, 4.0f, 4.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

// Hàm vẽ đài không lưu
void drawControlTower() {
    // Thân tháp
    glColor3f(0.8f, 0.8f, 0.7f);
    glPushMatrix();
    glTranslatef(-60.0f, 10.0f, 25.0f);
    glScalef(4.0f, 22.0f, 4.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Phòng điều khiển trên đỉnh
    glColor3f(0.6f, 0.8f, 0.9f); // Màu xanh kính
    glPushMatrix();
    glTranslatef(-60.0f, 22.0f, 25.0f);
    glScalef(8.0f, 4.0f, 8.0f);
    glutSolidCube(1.0);
    glPopMatrix();
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
            camX = planePosX - 12.0f * cos(radYaw);
            camY = planePosY + 4.0f;
            camZ = planePosZ - 12.0f * sin(radYaw);
            break;
        case 2:
            camX = planePosX + 1.5f * cos(radYaw);
            camY = planePosY + 0.5f;
            camZ = planePosZ + 1.5f * sin(radYaw);
            break;
        case 3:
            camX = planePosX - 25.0f * cos(radYaw);
            camY = planePosY + 8.0f;
            camZ = planePosZ - 25.0f * sin(radYaw);
            break;
        case 4:
            camX = planePosX;
            camY = planePosY + 40.0f;
            camZ = planePosZ + 0.1f;
            break;
    }

    gluLookAt(camX, camY, camZ, planePosX, planePosY, planePosZ, 0, 1, 0);

    // Mặt đất
    glColor3f(0.3f, 0.7f, 0.3f);
    glBegin(GL_QUADS);
        glVertex3f(-200.0f, -1.0f, -200.0f);
        glVertex3f(-200.0f, -1.0f,  200.0f);
        glVertex3f( 200.0f, -1.0f,  200.0f);
        glVertex3f( 200.0f, -1.0f, -200.0f);
    glEnd();

    // THÊM MỚI: Vẽ sân bay
    drawRunway();
    drawTerminal();
    drawControlTower();

    // Vẽ cây cối một cách ngẫu nhiên
    for (const auto& tree : trees) {
        glPushMatrix();
        // Di chuyển đến vị trí của cây và áp dụng tỷ lệ ngẫu nhiên
        glTranslatef(tree.x, -1.0f, tree.z);
        glScalef(tree.scale, tree.scale, tree.scale);

        // Vẽ thân cây
        glColor3f(0.4f, 0.2f, 0.1f);
        glPushMatrix();
            glRotatef(-90, 1, 0, 0);
            GLUquadric* trunk = gluNewQuadric();
            gluCylinder(trunk, 0.2, 0.2, 2.5, 8, 3);
            gluDeleteQuadric(trunk);
        glPopMatrix();

        // Vẽ tán lá
        glPushMatrix();
            glColor3f(0.0f, 0.6f, 0.0f);
            glTranslatef(0.0f, 2.5f, 0.0f);
            glutSolidSphere(1.2, 10, 10);
        glPopMatrix();

        glPopMatrix();
    }


    // Tòa nhà
    for (const Building& b : buildings) {
        glPushMatrix();
            glColor3f(0.5f, 0.5f, 0.6f);
            glTranslatef(b.x, b.height / 2.0f - 1.0f, b.z);
            glScalef(5.0f, b.height, 5.0f);
            glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Mây
    for (auto& c : clouds) {
        glPushMatrix();
            glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
            glTranslatef(c.x, c.y, c.z);
            glutSolidSphere(2.5, 10, 10);
        glPopMatrix();

        c.x += 0.005f;
        if (c.x > 200.0f) c.x = -200.0f;
    }

    // Máy bay
    glPushMatrix();
        glTranslatef(planePosX, planePosY, planePosZ);
        glRotatef(-planeYaw, 0, 1, 0);
        glRotatef(planePitch, 0, 0, 1);
        glRotatef(planeRoll, 1, 0, 0);
        plane();
    glPopMatrix();

    glutSwapBuffers();
}

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

void init() {
    glClearColor(0.7f, 0.85f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_position[] = { 10.0f, 50.0f, 10.0f, 0.0f };
    GLfloat light_ambient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light_diffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    srand(time(0));

    // Tạo 400 cây ngẫu nhiên để làm cho cảnh quan thưa hơn
    for (int i = 0; i < 400; ++i) {
        float x = (float)(rand() % 400 - 200); // Tọa độ x ngẫu nhiên từ -200 đến 200
        float z = (float)(rand() % 400 - 200); // Tọa độ z ngẫu nhiên từ -200 đến 200

        // THAY ĐỔI: Đảm bảo cây không mọc trên hoặc quá gần khu vực sân bay
        if (z > -15 && z < 40) { // Mở rộng khu vực cấm
            continue;
        }

        // Kích thước ngẫu nhiên từ 0.8 đến 1.3 lần kích thước gốc
        float scale = (float)(rand() % 50) / 100.0f + 0.8f;
        trees.push_back({x, z, scale});
    }

    // Mây
    for (int i = 0; i < 50; ++i) {
        clouds.push_back({(float)(rand() % 400 - 200), (float)(rand() % 20 + 15), (float)(rand() % 400 - 200)});
    }

    // Tòa nhà
    for (float i = -200; i <= 200; i += 50.0f) {
        for (float z = -200.0f; z <= 200.0f; z += 50.0f) {
            // THAY ĐỔI: Đảm bảo tòa nhà không mọc trên hoặc quá gần khu vực sân bay
            if (z > -15 && z < 40) { // Mở rộng khu vực cấm
                continue;
            }

            if (rand() % 2 == 0) {
                buildings.push_back({i, z, (float)((rand() % 15) + 5)});
            }
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Mo Phong May Bay - San Bay");

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
