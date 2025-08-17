//
//  main.cpp
//  VertexAnimation
//
//  Created by 이용규 on 8/16/25.
//

#include <YGLWindow.hpp>
YGLWindow* window;
#include <program.hpp>
Program shader;
#include <objreader.hpp>
#include <camera.hpp>

#include <filesystem>

struct Pose {
    std::vector<glm::vec3> p;
    std::vector<glm::vec3> n;
    
    Pose(const int nVertices) : p(nVertices), n(nVertices) {}
};
struct AnimatedObj {
private:
    std::string _fileName;
    float _cps; // one cycle per second
    std::vector<glm::u16vec3> faces;
    std::vector<Pose> poses;
    
public:
    AnimatedObj(const std::string& fileName) : _fileName(fileName) {}
    
    void writeYAObj() {
        if(poses.size() == 0) return;
        if(std::filesystem::exists(_fileName)) {
            std::cerr << _fileName << "File already exist. Fail to write." << std::endl;
            return;
        }
        
        std::ofstream ofs(_fileName);
        
        if(!ofs.is_open()) {
            std::cerr << "Cannot open " << _fileName << "!" << std::endl;
            return;
        }
        
        ofs << "f " << faces.size() << '\n';
        for (const auto& f : faces) {
            ofs << f.x << ' ' << f.y << ' ' << f.z << '\n';
        }
        ofs << "p " << poses.size() << ' ' << poses[0].p.size() << '\n';
        for (const Pose& pp : poses) {
            for (const auto& p : pp.p) {
                ofs << p.x << ' ' << p.y << ' ' << p.z << '\n';
            }
            for (const auto& n : pp.n) {
                ofs << n.x << ' ' << n.y << ' ' << n.z << '\n';
            }
        }
        
        ofs.close();
        
        std::cout << "Write " << _fileName << " Complete." << std::endl;
    }
    
    void loadAnimatedObjs() {
        for(int i = 1; i <=48; i++) {
            ObjData tmp;
            std::string target = "horse-gallop-00.obj";
            target[13] = '0' + i / 10;
            target[14] = '0' + i % 10;
            tmp.loadObject("horse-gallop", target.c_str());
            
            if(i == 1) {
                faces.resize(tmp.nElements3);
                for(int j = 0; j < tmp.nElements3; j++) faces[j] = tmp.elements3[j];
            }
            
            poses.push_back(Pose(tmp.nVertices));
            for(int j = 0; j < tmp.nVertices; j++) {
                poses[i-1].p[j] = tmp.vertices[j];
                poses[i-1].n[j] = tmp.syncedNormals[j];
            }
        }
        writeYAObj();
    }

    void loadYAObj() {
        std::ifstream ifs(_fileName);
        
        if(!ifs.is_open()) {
            std::cerr << "Cannot open " << _fileName << "! Read from scratch!" << std::endl;
            loadAnimatedObjs();
            return;
        }
        
        char type;
        while (ifs >> type) {
            switch (type) {
                case 'f':
                    int count;
                    ifs >> count;
                    faces.resize(count);
                    for(int i = 0; i < count; i++)
                        ifs>>faces[i].x>>faces[i].y>>faces[i].z;
                    break;
                case 'p':
                    int countPose, countVertices;
                    ifs >> countPose >> countVertices;
                    poses = std::vector<Pose>(countPose, Pose(countVertices));
                    for(int i = 0; i < countPose; i++) {
                        for(int j = 0; j < countVertices; j++)
                            ifs>>poses[i].p[j].x>>poses[i].p[j].y>>poses[i].p[j].z;
                        for(int j = 0; j < countVertices; j++)
                            ifs>>poses[i].n[j].x>>poses[i].n[j].y>>poses[i].n[j].z;
                    }
                    break;
                    
                default:
                    break;
            }
        }
        
        std::cout << _fileName << " read completed." << std::endl;
    }
    
    void setCPS(const float& cps) { _cps = cps; }
    
    int nextSaved = 1;
    GLuint vao = -1, posvbo[2], norvbo[2], veo;
    void render(const float& t) {
        float period = glm::fract(t/_cps)*poses.size();
        int cur  = floor(period);
        int next = (cur + 1) % poses.size();
        
        shader.use();
        shader.setUniform("interTime", glm::min(1.f, glm::max(0.f, (period-cur)/(next-cur))));
        
        if(nextSaved == cur) {
            nextSaved = next;
        }
        
        if(vao == -1) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(2, posvbo);
            glBindBuffer(GL_ARRAY_BUFFER, posvbo[0]);
            glBufferData(GL_ARRAY_BUFFER, poses[0].p.size()*sizeof(glm::vec3), poses[0].p.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, posvbo[1]);
            glBufferData(GL_ARRAY_BUFFER, poses[0].p.size()*sizeof(glm::vec3), poses[0].p.data(), GL_DYNAMIC_DRAW);
            glGenBuffers(2, norvbo);
            glBindBuffer(GL_ARRAY_BUFFER, norvbo[0]);
            glBufferData(GL_ARRAY_BUFFER, poses[0].n.size()*sizeof(glm::vec3), poses[0].n.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, norvbo[1]);
            glBufferData(GL_ARRAY_BUFFER, poses[0].n.size()*sizeof(glm::vec3), poses[0].n.data(), GL_DYNAMIC_DRAW);
            glGenBuffers(1, &veo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size()*sizeof(glm::u16vec3), faces.data(), GL_STATIC_DRAW);
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, posvbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, poses[cur].p.size()*sizeof(glm::vec3), poses[cur].p.data());
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, norvbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, poses[cur].n.size()*sizeof(glm::vec3), poses[cur].n.data());
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, posvbo[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, poses[next].p.size()*sizeof(glm::vec3), poses[next].p.data());
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, norvbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, poses[next].n.size()*sizeof(glm::vec3), poses[next].n.data());
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
        glDrawElements(GL_TRIANGLES, faces.size()*3, GL_UNSIGNED_SHORT, 0);
    }
};
AnimatedObj animatedObj("HorseGallop.yao");

void init() {
    shader.loadShader("VertexAnimation/va.vert", "VertexAnimation/va.frag");
    animatedObj.loadYAObj();
    animatedObj.setCPS(3);
    
    camera.setPosition({0, 0, 5});
    camera.glfwSetCallbacks(window->getGLFWWindow());
}

float t = 0;
void render() {
    glViewport(0, 0, window->width(), window->height());
    glClearColor(0, 0, 0.3, 0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 V = camera.lookAt();
    shader.setUniform("NormalMatrix", glm::mat3(V[0], V[1], V[2]));
    shader.setUniform("MVP", camera.perspective(window->aspect(), 0.1f, 1000.f)*V);
    t = glfwGetTime();
    animatedObj.render(t);
}

int main(int argc, const char * argv[]) {
    window = new YGLWindow(640, 480, "Vertex Animation");
    window->mainLoop(init, render);
    
    
    
    return 0;
}
