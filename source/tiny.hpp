#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>

#include "../lib/olcPGE.hpp"
#include "../lib/json.hpp"

namespace tiny
{
class Set;
class Component;
class Node;
class NodeTree;
class Layer;
class ObjectLayer;
class TileLayer;
class Level;
class PixelGame;
class TinyEngine;

olc::Pixel hexToPixel(std::string hex);

class Set{
private:
    int                             firstgid = 0;
    int                             columns = 0;
    olc::vi2d                       imageRes = {0, 0};
    olc::vi2d                       tileRes = {0, 0};
    std::unique_ptr<olc::Sprite>    sprite;
    std::unique_ptr<olc::Decal>     decal;

public:
    Set(int gid, int col, olc::vi2d im, olc::vi2d ti, std::string path);
    int getFirstgid();
    int getColumns();
    olc::vi2d getImageRes();
    olc::vi2d getTileRes();
    olc::Sprite* getSprite();
    olc::Decal* getDecal();
};

class Component{
private:
    bool                m_active;
    int                 m_id;
    std::vector<int>    m_childrenID;
/*
    olc::vf2d               m_position;
    olc::vf2d               m_velocity;
*/

public:
    Component(nlohmann::json description);
    virtual bool ComponentUpdate(float fElapsedTime);
    virtual void ComponentDraw(PixelGame& pixelRef);
    // virtual bool SetLocation(olc::vf2d newLocation);
    // virtual bool AddLocation(olc::vf2d deltaLocation);
    // virtual bool AddForce(olc::vf2d velocity);
    virtual bool Signal(bool state);
    // virtual bool SetProperties(nlohmann::json* data);
    // virtual ~Component();
};

class Node{};

class NodeTree{};

class Layer{
private:
    std::string m_name  = "Default";
    int         m_id    = 0;

protected:
    TinyEngine* m_engine;
    PixelGame*  m_game;
    Level*      m_level;
    olc::Pixel  m_tint = olc::WHITE;

public:
    Layer(nlohmann::json description, TinyEngine* engine, PixelGame* game, Level* level);
    Layer() = default;
    void DisplayLayerName();
    bool LayerUpdate(float fElapsedTime);
    virtual void LayerDraw();
};

class ObjectLayer : public Layer{
private:
    std::vector<Component*> m_components;

public:
    ObjectLayer(nlohmann::json description, TinyEngine* engine, PixelGame* game, Level* level);
    void LayerDraw();
};

class TileLayer : public Layer{
private:
    std::vector<std::vector<int>>   m_mapData;
    int                             m_height{};
    int                             m_width{};

public:
    TileLayer(nlohmann::json description, TinyEngine* engine, PixelGame* game, Level* level);
    void LayerDraw();

};

class Level{
private:
    std::vector<std::unique_ptr<Layer>> m_layers;
    std::vector<std::unique_ptr<Set>>   m_sets;
    nlohmann::json                      m_description;
    TinyEngine*                         m_engine;
    PixelGame*                          m_pixelGame;

    void AddLayer(nlohmann::json description);
    void AddSet(nlohmann::json description);

    template <typename T>
    void AddItem(nlohmann::json description);

public:
    Level(nlohmann::json description, TinyEngine* engine, PixelGame* pixelGame);
    bool Update(float fElapsedTime);
    void Draw();
    int getSetIndex(int tile);
    Set* getSet(int index);
    void printSets();
};

class PixelGame : public olc::PixelGameEngine {
    private:
        TinyEngine* m_engine;
        std::string m_firstLevel;

    public:
        PixelGame(TinyEngine* engine, std::string level);
        ~PixelGame();
        bool OnUserCreate();
        bool OnUserUpdate(float fElapsedTime);
        bool OnUserDestroy();
};

class TinyEngine {
private:
    // std::map<int, std::unique_ptr<Component>>           m_worldComponents;
    std::unique_ptr<Level>                              m_level;
    std::unique_ptr<PixelGame>                          m_pixelGame;
    olc::vi2d                                           m_resolution;
    std::map<std::string, std::function<Component*()>>  m_objectFactory;
    std::string                                         m_name;
    bool                                                m_constructed{};

public:
    TinyEngine(olc::vi2d resolution, std::string name);
    ~TinyEngine();
    void Run(std::string level);
    void LoadLevel(std::string path);
    Level* GetLevel();
    Component* GetComponentByID(int id);
    void AddComponentByID(nlohmann::json& description);
    
    std::string GetName();
    void AddDefinition(std::string key, std::function<Component*()> lambda);
    std::function<Component*()> GetDefinition(std::string key);
    // ComponentFactory& Getfactory();
};
}