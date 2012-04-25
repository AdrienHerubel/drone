#include "GLCommon.hpp"
#include "Camera.hpp"
#include "HighResolutionTimer.hpp"
#include "Transform.hpp"
#include "LinearAlgebra.hpp"
#include "GLUtils.hpp"
#include "GPUPrimitive.hpp"
#include "ViewerShaders.hpp"
#include "ShaderGLSL.hpp"

#include <drone_scene/drone_scene.hpp>
#include <drone/drn_reader.h>


#include <GL/glut.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

void glutRenderBitmapString(float x, float y, void *font, char *string);

#define DRN_DBG_LVL 1
#if DRN_DBG_LVL == 0
#define DRN_DBG_LVL1(EXP)
#define DRN_DBG_LVL2(EXP)
#elif DRN_DBG_LVL == 1
#include <iostream>
#define DRN_DBG_LVL1(EXP) EXP
#define DRN_DBG_LVL2(EXP)
#elif DRN_DBG_LVL > 1
#include <iostream>
#define DRN_DBG_LVL1(EXP) EXP
#define DRN_DBG_LVL2(EXP) EXP
#endif


namespace viewer
{
    void usage();

    struct View
    {
        Camera camera;
        uint32_t width;
        uint32_t height;
    };

    struct Sampler
    {
        GLuint textureId;
    };

    struct ForwardShader
    {
        GLuint program;
        GLuint projectionLocation;
        GLuint viewLocation;
        GLuint objectLocation;
        GLuint diffuseColorLocation;
        GLuint specularPowerLocation;
        ShaderGLSL shader;
    };

    struct Render
    {
        // Shaders
        ForwardShader forwardShader;
        // Per primitive sorted arrays
        std::vector<int32_t> geometryLoaded;
        std::vector<int32_t> geometryUpdated;
        std::vector<GPUPrimitive> primitives;
        std::vector<uint32_t> sceneIdx;
        std::vector<uint64_t> meshIdx;
        std::vector<uint32_t> samplerIdx;
        std::vector<uint64_t> primitiveSIdx;
        uint64_t numTransparentPrimitives;
        // Textures
        std::vector<Sampler> samplers;
    };

    struct Geometry
    {
        std::vector<drn_t> caches;
        std::vector<drn_scene::Scene> scenes;
    };

    struct GUIStates
    {
        bool panLock;
        bool turnLock;
        bool zoomLock;
        uint32_t lockPositionX;
        uint32_t lockPositionY;
        uint32_t camera;
        double time;
        bool playing;
        TimerStruct timeCounterFPS;
        uint32_t fpsCounter;
        uint32_t fpsCounterLast;
        TimerStruct timeCounterAnim;
        static const float MOUSE_PAN_SPEED = 0.001f;
        static const float MOUSE_ZOOM_SPEED = 0.05f;
        static const float MOUSE_TURN_SPEED = 0.005f;
    };
    const float GUIStates::MOUSE_PAN_SPEED;
    const float GUIStates::MOUSE_ZOOM_SPEED;
    const float GUIStates::MOUSE_TURN_SPEED;

    struct Viewer
    {
        View view;
        Render render;
        GUIStates guiStates;
        Geometry geometry;
    };
    Viewer GLOBAL_VIEWER;

    void defaultView(Viewer & viewer)
    {
        viewer.view.width = 512;
        viewer.view.height = 512;
    }

    void defaultGUIStates(Viewer & viewer)
    {
        viewer.guiStates.panLock = false;
        viewer.guiStates.turnLock = false;
        viewer.guiStates.zoomLock = false;
        viewer.guiStates.lockPositionX = 0;
        viewer.guiStates.lockPositionY = 0;
        viewer.guiStates.camera = 0;
        viewer.guiStates.time = 0.0;
        viewer.guiStates.playing = false;
        GetHighResolutionTime(&viewer.guiStates.timeCounterFPS);
        viewer.guiStates.fpsCounter = 0;
        GetHighResolutionTime(&viewer.guiStates.timeCounterAnim);
    }

    void defaultRender(Viewer & viewer)
    {
        viewer.render.forwardShader.shader.setSource(ShaderGLSL::VERTEX_SHADER, strlen(VSFWD), VSFWD);
        viewer.render.forwardShader.shader.setSource(ShaderGLSL::FRAGMENT_SHADER, strlen(FSFWD), FSFWD);
        viewer.render.forwardShader.shader.initGPU();
        viewer.render.forwardShader.program = viewer.render.forwardShader.shader.shaderProgramObject();
        viewer.render.forwardShader.projectionLocation = glGetUniformLocation(viewer.render.forwardShader.program, "Projection");
        viewer.render.forwardShader.viewLocation = glGetUniformLocation(viewer.render.forwardShader.program, "View");
        viewer.render.forwardShader.objectLocation = glGetUniformLocation(viewer.render.forwardShader.program, "Object");
        viewer.render.forwardShader.diffuseColorLocation = glGetUniformLocation(viewer.render.forwardShader.program, "DiffuseColor");
        viewer.render.forwardShader.specularPowerLocation = glGetUniformLocation(viewer.render.forwardShader.program, "SpecularPower");
    }

    void reshapeCallback(int newwidth, int newheight)
    {
        const uint32_t numGBufferLayers = 2;
        glViewport( 0, 0, newwidth, newheight  );
        GLOBAL_VIEWER.view.camera.setViewport(0, 0, newwidth, newheight);
        GLOBAL_VIEWER.view.width = newwidth;
        GLOBAL_VIEWER.view.height = newheight;
    }

    void idleCallback(void)
    {
    	TimerStruct newTime; GetHighResolutionTime(&newTime);
    	if (ConvertTimeDifferenceToSec( &newTime, &GLOBAL_VIEWER.guiStates.timeCounterFPS ) > 1.0)
        {
          DRN_DBG_LVL2(std::cout<<"fps:"<<GLOBAL_VIEWER.guiStates.fpsCounter<<std::endl;);
          GetHighResolutionTime(&GLOBAL_VIEWER.guiStates.timeCounterFPS);
          GLOBAL_VIEWER.guiStates.fpsCounterLast = GLOBAL_VIEWER.guiStates.fpsCounter;
          GLOBAL_VIEWER.guiStates.fpsCounter = 0;
      }
      if (GLOBAL_VIEWER.guiStates.playing)
      {
          if (ConvertTimeDifferenceToSec( &newTime, &GLOBAL_VIEWER.guiStates.timeCounterAnim ) > 1.0/30.0)
          {
             DRN_DBG_LVL2(std::cout<<"time:"<<GLOBAL_VIEWER.guiStates.time<<std::endl;);
             GetHighResolutionTime(&GLOBAL_VIEWER.guiStates.timeCounterAnim);
             GLOBAL_VIEWER.guiStates.time += 1.0;
         }
     }
     glutPostRedisplay();
 }

 void keyboardCallback(unsigned char c, int x, int y){
   switch(c)
   {
    case 27 :
    exit(0);
    break;
    case ' ' :
    GLOBAL_VIEWER.guiStates.playing = !GLOBAL_VIEWER.guiStates.playing;
    break;
    case '1':
    GLOBAL_VIEWER.guiStates.camera = 1;
    break;
    case '0':
    GLOBAL_VIEWER.guiStates.camera = 0;
    break;
    case '+':
    break;
    case '-':
    break;
    break;
    default:
    DRN_DBG_LVL1(std::cout << "Key " << (int) c << ":"<< c<< std::endl;);
    break;
}
glutPostRedisplay();
}

void keyboardSpecialCallback(int c, int x, int y)
{
	switch(c)
    {
        case GLUT_KEY_UP :
        break;
        case GLUT_KEY_DOWN :
        break;
        case GLUT_KEY_LEFT :
        GLOBAL_VIEWER.guiStates.time -= 1.0;
        break;
        case GLUT_KEY_RIGHT :
        GLOBAL_VIEWER.guiStates.time += 1.0;
        break;
        case GLUT_KEY_PAGE_UP :
        break;
        case GLUT_KEY_PAGE_DOWN :
        break;
        case GLUT_KEY_HOME :
        GLOBAL_VIEWER.guiStates.time = 0.0;
        break;
        default:
        DRN_DBG_LVL1(std::cout << "Special Key " << (int) c << std::endl;);
        break;
    }
    glutPostRedisplay();
}

void mouseCallback(int button, int state, int x, int y)
{
	GLOBAL_VIEWER.guiStates.lockPositionX = x;
	GLOBAL_VIEWER.guiStates.lockPositionY = y;
	if(button == int(GLUT_LEFT_BUTTON) && state == int(GLUT_DOWN))
    {
      GLOBAL_VIEWER.guiStates.turnLock = true;
  }
  else if (button == int(GLUT_RIGHT_BUTTON) && state == int(GLUT_DOWN))
  {
      GLOBAL_VIEWER.guiStates.zoomLock = true;
  }
  else if (button == int(GLUT_MIDDLE_BUTTON) && state == int(GLUT_DOWN))
  {
      GLOBAL_VIEWER.guiStates.panLock = true;
  }
  else if(button == int(GLUT_LEFT_BUTTON) && state == int(GLUT_UP))
  {
      GLOBAL_VIEWER.guiStates.turnLock = false;
  }
  else if (button == int(GLUT_RIGHT_BUTTON) && state == int(GLUT_UP))
  {
      GLOBAL_VIEWER.guiStates.zoomLock = false;
  }
  else if (button == int(GLUT_MIDDLE_BUTTON) && state == int(GLUT_UP))
  {
      GLOBAL_VIEWER.guiStates.panLock = false;
  }
  glutPostRedisplay();
}

void motionCallback(int x, int y)
{
	if (! (glutGetModifiers() & GLUT_ACTIVE_ALT))
        return;
    DRN_DBG_LVL2(std::cout << "Mouse motion lock "	<< x << " "<< y << std::endl;);
    int32_t diffLockPositionX = x - GLOBAL_VIEWER.guiStates.lockPositionX;
    int32_t diffLockPositionY = y - GLOBAL_VIEWER.guiStates.lockPositionY;
    if (GLOBAL_VIEWER.guiStates.zoomLock)
    {
      float zoomDir = (diffLockPositionX > 0) ? -1.f	: 1.f;
      GLOBAL_VIEWER.view.camera.zoom(zoomDir * GUIStates::MOUSE_ZOOM_SPEED);
  }
  else if (GLOBAL_VIEWER.guiStates.turnLock)
  {
      GLOBAL_VIEWER.view.camera.turn(1.f * diffLockPositionY * GUIStates::MOUSE_TURN_SPEED,
          1.f * diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

  }
  else if (GLOBAL_VIEWER.guiStates.panLock)
  {
      GLOBAL_VIEWER.view.camera.pan(diffLockPositionX * GUIStates::MOUSE_PAN_SPEED,
       diffLockPositionY * GUIStates::MOUSE_PAN_SPEED);
  }
  GLOBAL_VIEWER.guiStates.lockPositionX = x;
  GLOBAL_VIEWER.guiStates.lockPositionY = y;
  glutPostRedisplay();
}

void displayCallback(void)
{
    // Pointers to global app
    Viewer * __restrict__ viewer = & GLOBAL_VIEWER;
    Render *  __restrict__ render = & viewer->render;
    Geometry *  __restrict__ geometry = & viewer->geometry;
    View *  __restrict__ view = & viewer->view;
    GUIStates *  __restrict__ states = & viewer->guiStates;

    // Resolve pointers for primitives
    GPUPrimitive * ps = & render->primitives[0];
    uint64_t * srtid = & render->primitiveSIdx[0];
    int32_t * gl = & render->geometryLoaded[0];
    int32_t * gu = & render->geometryUpdated[0];
    uint32_t * sid =  & render->sceneIdx[0];
    uint64_t * mid = & render->meshIdx[0];
    uint32_t * smid = & render->samplerIdx[0];
    uint64_t firstOpaque = render->numTransparentPrimitives;
    uint64_t lastOpaque = render->primitives.size();
    uint64_t firstTrans = 0;
    uint64_t lastTrans = render->numTransparentPrimitives;

    // Compute orthographic projection for fullscreen quad
    float orthoProj[16];
    ortho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0, orthoProj);
    // Compute inverse of the projection matrix
    float projection[16];
    float projectionT[16];
    float worldToView[16];
    mat4fCopy(projection, view->camera.perspectiveProjection());
    mat4fTranspose(projection, projectionT);
    float worldToViewT[16];
    float cameraPosition[4];
    float viewProjection[16];
    float iviewProjection[16];
    // Choose camera
    if (states->camera == 0) // Default cam
    {
        // Set view
        mat4fCopy(worldToView, view->camera.worldToView());
        mat4fTranspose(worldToView, worldToViewT);
        vec4fCopy(cameraPosition, view->camera.position());
    }
    else if (states->camera == 1) // Stored cam
    {
        const drn_scene::Camera * cam = & geometry->scenes[0].cameras[0];
        uint32_t frame = drn_scene::timeToFrame(states->time,
         geometry->scenes[0].timeRange.startFrame,
         geometry->scenes[0].timeRange.endFrame,
         geometry->scenes[0].timeRange.subFrames);
        float viewTransform[16];
        lookAt(cam->dynamicData[frame].eye, cam->dynamicData[frame].center, cam->dynamicData[frame].up, viewTransform);
        // Set view
        mat4fTranspose(viewTransform, worldToView);
        mat4fTranspose(worldToView, worldToViewT);
        vec4fCopy(cameraPosition, cam->dynamicData[frame].eye);
    }
    mat4fMul( projectionT, worldToViewT, viewProjection);
    mat4fInverse(viewProjection, iviewProjection);

    //
    // Update Geometry
    //
    uint64_t numTriangles = 0;
    for (uint64_t i = 0; i != render->primitives.size(); ++i)
    {
        uint64_t id = srtid[i];
        GPUPrimitive * p = ps + id;
        drn_scene::Scene * s = & geometry->scenes[sid[id]];
        drn_scene::Mesh * m = s->meshes + mid[id];
        if (!m->numTriangles)
          continue;
        numTriangles += m->numTriangles;
        uint32_t frame = drn_scene::timeToFrame(states->time,
        s->timeRange.startFrame,
        s->timeRange.endFrame,
        s->timeRange.subFrames);
        // If primitive is not loaded
        if (! gl[id] )
        {
            glBindVertexArray(0);
            GPUPrimitive_updateIndexBuffer(p,
             m->numTriangles * sizeof(int32_t) * 3,
             m->triangleList);
            p->indexCount = m->numTriangles;
            GPUPrimitive_updateBuffer(p,
              2,
              m->numHwVertices * 3 * sizeof(float),
              m->hwUVs);
            GPUPrimitive_updateBuffer(p,
              0,
              m->numHwVertices * 3 * sizeof(float),
              m->dynamicData[frame].hwNormals);
            gl[id] = 1;
            GPUPrimitive_updateBuffer(p,
              1,
              m->numHwVertices * 3 * sizeof(float),
              m->dynamicData[frame].hwNormals);
            gl[id] = 1;
        }
        // If primitive is not up to date
        if (gu[id] != (int) frame)
        {
            glBindVertexArray(0);
            GPUPrimitive_updateBuffer(p,
              0,
              m->numHwVertices * 3 * sizeof(float),
              m->dynamicData[frame].hwVertices);
            GPUPrimitive_updateBuffer(p,
             1,
             m->numHwVertices * 3 * sizeof(float),
             m->dynamicData[frame].hwNormals);
            gu[id] = frame;
        }
        ++p;
    }

    // Clear the front buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render->forwardShader.shader.enable();
    GLuint program = render->forwardShader.shader.shaderProgramObject();
    GLuint location = glGetUniformLocation(program, "Projection");
    glUniformMatrix4fv(location, 1, 0, projection);
    location = glGetUniformLocation(program, "View");
    glUniformMatrix4fv(location, 1, 0, worldToView);
    location = glGetUniformLocation(program, "Object");

    //
    // Opaque Object Rendering
    //
    for (uint64_t i = firstOpaque; i != lastOpaque; ++i)
    {
        uint64_t id = srtid[i];
        GPUPrimitive * p = ps + id;
        drn_scene::Scene * s = & geometry->scenes[sid[id]];
        drn_scene::Mesh * m = s->meshes + mid[id];
        uint64_t numDagNodes = s->numDagNodes;
        uint64_t did = m->dagNodeId;
        uint32_t matid = s->dagNodes[did].materialId;
        const drn_scene::Material * mat = s->materials + matid;
        uint32_t frame = drn_scene::timeToFrame(states->time,
         s->timeRange.startFrame,
         s->timeRange.endFrame,
         s->timeRange.subFrames);
        glBindTexture(GL_TEXTURE_2D, render->samplers[smid[id]].textureId);
        glUniformMatrix4fv(render->forwardShader.objectLocation, 1, 0, s->dagNodeStates[numDagNodes*frame+did].otwTransform);
        glUniform4fv(render->forwardShader.diffuseColorLocation, 1, mat->diffuseColor );
        glUniform1f(render->forwardShader.specularPowerLocation, mat->specularPower );
        // Render primitive
        GPUPrimitive_render(p);
        ++p;
    }

    //
    // TRANSPARENT OBJECT RENDERING
    //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    // Foreach primitive
    for (uint64_t i = firstTrans; i != lastTrans; ++i)
    {
        uint64_t id = srtid[i];
        GPUPrimitive * p = ps + id;
        drn_scene::Scene * s = & geometry->scenes[sid[id]];
        drn_scene::Mesh * m = s->meshes + mid[id];
        uint64_t numDagNodes = s->numDagNodes;
        uint64_t did = m->dagNodeId;
        uint32_t matid = s->dagNodes[did].materialId;
        const drn_scene::Material * mat = s->materials + matid;
        uint32_t frame = drn_scene::timeToFrame(states->time,
         s->timeRange.startFrame,
         s->timeRange.endFrame,
         s->timeRange.subFrames);
        glBindTexture(GL_TEXTURE_2D, render->samplers[smid[id]].textureId);
        glUniformMatrix4fv(render->forwardShader.objectLocation, 1, 0, s->dagNodeStates[numDagNodes*frame+did].otwTransform);
        glUniform4fv(render->forwardShader.diffuseColorLocation, 1, mat->diffuseColor );
        glUniform1f(render->forwardShader.specularPowerLocation, mat->specularPower );
        // Render primitive
        GPUPrimitive_render(p);
        ++p;
    }

    GLOBAL_VIEWER.render.forwardShader.shader.disable();

	//
	// RENDERING UTILS
	//
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);
    glMatrixMode(GL_MODELVIEW);

    drawAxes(GLOBAL_VIEWER.view.camera.worldToView());
    glViewport( 0, 0, GLOBAL_VIEWER.view.width, GLOBAL_VIEWER.view.height	 );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    char hudDisplayString[256];
    sprintf(hudDisplayString, "FPS : %d | Time : %f | %Lu Triangles ", GLOBAL_VIEWER.guiStates.fpsCounterLast, GLOBAL_VIEWER.guiStates.time, numTriangles);
    glutRenderBitmapString(-1.0, -0.99, GLUT_BITMAP_HELVETICA_10, hudDisplayString);

    GLenum err = glGetError();
    if(err!=GL_NO_ERROR)
    {
      std::cerr << "Erreur GL :" << std::endl;
      std::cerr << gluErrorString(err) << std::endl;
  }
  glutSwapBuffers();
  ++GLOBAL_VIEWER.guiStates.fpsCounter;
}

void defaultStatesGL()
{
	glClearColor(0.0,0.0,0.0,1.0f);
	glEnable(GL_DEPTH_TEST);
}

void usage()
{
    std::cout << "viewer <cache> <cache_dir>" << std::endl;
}

struct TransparencySorter
{
    TransparencySorter(const Render * r, const Geometry * g) : r(r), g(g) {}
    bool operator()(int64_t a, int64_t b)
    {
        uint64_t amid = r->meshIdx[a];
        uint64_t asid = r->sceneIdx[a];
        const drn_scene::Scene * as = & g->scenes[asid];
        uint64_t adid = as->meshes[amid].dagNodeId;
        uint32_t amtid = as->dagNodes[adid].materialId;
        const float * at = as->materials[amtid].transparency;
        uint64_t bmid = r->meshIdx[b];
        uint64_t bsid = r->sceneIdx[b];
        const drn_scene::Scene * bs = & g->scenes[bsid];
        uint64_t bdid = bs->meshes[bmid].dagNodeId;
        uint32_t bmtid = bs->dagNodes[bdid].materialId;
        const float * bt = bs->materials[bmtid].transparency;
        return at[0] + at[1] + at[2] > bt[0] + bt[1] + bt[2];
    };
    const Render * r;
    const Geometry * g;
};

void addDRNScene(Viewer & viewer, const char * fileName)
{
    drn_t cache;
    int32_t status = drn_open(&cache, fileName, DRN_READ_MMAP);
    if (status)
    {
        std::cerr << "Invalid file/dir " << fileName << std::endl;
        usage();
        exit(1);
    }
    if (strcmp(drn_get_description(&cache), DRN_SCENE_CACHE_DESCRIPTION) != 0)
    {
        std::cerr << "Not a drn_scene cache " << fileName << std::endl;
        usage();
        exit(1);
    }
    drn_map_id_t sceneChunkTypeTagId = drn_get_map_id(&cache, DRN_SCENE_CHUNK_TYPE_TAG);
    drn_chunk_id_t sceneEntryId;
    if (drn_get_matching_chunks(&cache, sceneChunkTypeTagId, DRN_SCENE_CHUNK_TAG_TYPE_VALUE, 1, &sceneEntryId) != 1)
    {
        std::cerr << "Invalid drn_scene cache " << fileName << std::endl;
        usage();
        exit(1);
    }
    drn_scene::Scene scene;
    drn_scene::resolveScene(&cache, &scene, drn_get_chunk(&cache, sceneEntryId));
    viewer.geometry.caches.push_back(cache);
    viewer.geometry.scenes.push_back(scene);
    viewer.render.geometryLoaded.resize(viewer.render.geometryLoaded.size()+scene.numMeshes, 0);
    viewer.render.geometryUpdated.resize(viewer.render.geometryLoaded.size()+scene.numMeshes, -1);
    viewer.render.sceneIdx.resize(viewer.render.geometryLoaded.size()+scene.numMeshes, viewer.geometry.scenes.size() - 1);
    viewer.render.meshIdx.resize(viewer.render.geometryLoaded.size()+scene.numMeshes);
    viewer.render.samplerIdx.resize(viewer.render.geometryLoaded.size()+scene.numMeshes);
    viewer.render.primitives.resize(viewer.render.primitives.size()+scene.numMeshes);
    viewer.render.samplers.resize(viewer.render.samplers.size()+scene.numMaterials);
    for (uint32_t i = viewer.render.primitives.size()-scene.numMeshes, j=0, n = viewer.render.primitives.size(); i != n; ++i, ++j)
    {
        GPUPrimitive_setup(&viewer.render.primitives[i]);
        viewer.render.sceneIdx[i] = viewer.geometry.scenes.size() - 1;
        viewer.render.meshIdx[i] = j;
        viewer.render.samplerIdx[i] = viewer.render.samplers.size()-scene.numMaterials + scene.dagNodes[scene.meshes[j].dagNodeId].materialId;
    }
    for (uint32_t i = viewer.render.samplers.size()-scene.numMaterials, j = 0, n = viewer.render.samplers.size(); i != n; ++i,++j)
    {
        drn_scene::Texture * t = scene.materials[j].diffuseTexture;
        if (!t)
        {
            viewer.render.samplers[i].textureId = 0;
            continue;
        }
        GLenum channels;
        if (t->ncomp == 3)
        {
            channels = GL_RGB;
        }
        else if (t->ncomp == 1)
        {
            channels = GL_LUMINANCE;
        }
        else
        {
            channels = GL_RGBA;
        }
        glGenTextures(1, & viewer.render.samplers[i].textureId);
        glBindTexture(GL_TEXTURE_2D, viewer.render.samplers[i].textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, channels, t->width, t->height, 0, channels, GL_UNSIGNED_BYTE, t->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    viewer.render.primitiveSIdx.clear();
    viewer.render.primitiveSIdx.resize(viewer.render.primitives.size());
    for(uint64_t i = 0; i < viewer.render.primitiveSIdx.size(); ++i)
        viewer.render.primitiveSIdx[i] = i;

    TransparencySorter ts(&viewer.render, &viewer.geometry);
    std::sort(viewer.render.primitiveSIdx.begin(), viewer.render.primitiveSIdx.end(), ts);
    uint64_t numTransparentPrimitives = 0;
    for (uint64_t i = 0; i < viewer.render.primitiveSIdx.size(); ++i)
    {
        uint64_t id  = viewer.render.primitiveSIdx[i];
        uint64_t amid = viewer.render.meshIdx[id];
        uint64_t asid = viewer.render.sceneIdx[id];
        const drn_scene::Scene * as = & viewer.geometry.scenes[asid];
        uint64_t adid = as->meshes[amid].dagNodeId;
        uint32_t amtid = as->dagNodes[adid].materialId;
        const float * at = as->materials[amtid].transparency;
        if (at[0] + at[1] + at[2] > 0.0001)
        {
            ++numTransparentPrimitives;
        }
    }
    viewer.render.numTransparentPrimitives = numTransparentPrimitives;
}


}; // namespace viewer


int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(512, 512);

    if(glutCreateWindow("Viewer") == 0)
        return 1;

    GLenum err = glewInit();
    if (GLEW_OK != err){
        std::cout << "Error : " << glewGetErrorString(err) << std::endl;
        return false;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!strstr(argv[i], ".drn"))
        {
            DIR * dir;
            dirent * entry;
            dir = opendir(argv[i]);
            if(!dir)
            {
                std::cerr << "Invalid file/dir " << argv[i] << std::endl;
                viewer::usage();
                exit(1);
            }
            while((entry = readdir(dir)))
            {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strstr(entry->d_name, ".drn"))
                {
                    DRN_DBG_LVL1(std::cout << "Add file " << entry->d_name << std::endl;);
                    addDRNScene(viewer::GLOBAL_VIEWER, (std::string(argv[i]) + entry->d_name).c_str());
                }
            }
            closedir(dir);
        }
        else
        {
            DRN_DBG_LVL1(std::cout << "Add file " << argv[i] << std::endl;);
            addDRNScene(viewer::GLOBAL_VIEWER, argv[i]);
        }
    }

    viewer::defaultStatesGL();
    viewer::defaultGUIStates(viewer::GLOBAL_VIEWER);
    viewer::defaultView(viewer::GLOBAL_VIEWER);
    viewer::defaultRender(viewer::GLOBAL_VIEWER);

    glutReshapeFunc(viewer::reshapeCallback);
    glutDisplayFunc(viewer::displayCallback);
    glutIdleFunc(viewer::idleCallback);
    glutKeyboardFunc(viewer::keyboardCallback);
    glutSpecialFunc(viewer::keyboardSpecialCallback);
    glutMouseFunc(viewer::mouseCallback);
    glutMotionFunc(viewer::motionCallback);

    viewer::reshapeCallback(256, 256);
    glutMainLoop();

    return 0;
}

void glutRenderBitmapString(float x, float y, void *font, char *string)
{
    char *c;
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++)
        glutBitmapCharacter(font, *c);
}


