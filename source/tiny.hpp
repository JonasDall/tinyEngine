#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>

#include "../lib/olcPGE.hpp"
#include "../lib/json.hpp"

namespace tiny
{
class Level;
class TinyEngine;
class PixelGame;
class Component;
struct int2d;
struct float2d;

struct int2d{
int x;
int y;

int2d operator + (const int2d& a) const
{
    return int2d{ x+a.x, y+a.y };
}

int2d operator - (const int2d& a) const
{
    return int2d{ x-a.x, y-a.y };
}

int2d operator + (const float2d& a) const
{
    return int2d{ x+(int)a.x, y+(int)a.y };
}

int2d operator - (const float2d& a) const
{
    return int2d{ x-(int)a.x, y-(int)a.y };
}

};

struct float2d{
float x;
float y;

float2d operator + (const float2d& a) const
{
    return float2d{ x+a.x, y+a.y };
}

float2d operator - (const float2d& a) const
{
    return float2d{ x-a.x, y-a.y };
}

float2d operator + (const float2d& a) const
{
    return float2d{ x+(float)a.x, y+(float)a.y };
}

float2d operator - (const float2d& a) const
{
    return float2d{ x-(float)a.x, y-(float)a.y };
}

};

class Set{

private:
int firstgid = 0;
int columns = 0;
int2d imageRes = {0, 0};
int2d tileRes = {0, 0};
std::unique_ptr<olc::Sprite> sprite;
std::unique_ptr<olc::Decal> decal;

public:
Set(int gid, int col, int2d im, int2d ti, std::string path);

int getFirstgid();
int getColumns();
int2d getImageRes();
int2d getTileRes();
olc::Sprite* getSprite();
olc::Decal* getDecal();
};

class Component
{
private:
    bool                m_active;
    int                 m_id;
    std::vector<int>    m_childrenID;
    nlohmann::json      m_description;

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

class Layer
{
private:
    std::string m_name  = "Default";
    int         m_id    = 0;
    TinyEngine* m_engine;

public:
    Layer(nlohmann::json description, TinyEngine* engine);
    Layer() = default;
    void DisplayLayerName();
    bool LayerUpdate(float fElapsedTime);
    virtual void LayerDraw(PixelGame* pixelRef);
};

class ObjectLayer : public Layer
{
private:
    std::vector<Component*> m_components;

public:
    ObjectLayer(nlohmann::json description, TinyEngine* engine);
    void LayerDraw(PixelGame* pixelRef);
};

class TileLayer : public Layer
{
private:
    std::vector<int>                m_mapData;
    std::string                     m_imagePath{};
    int                             m_tileSize{};
    int                             m_columns{};
    int                             m_rows{};
    int                             m_textureColumns{};
    int                             m_textureRows{};
    std::unique_ptr<olc::Sprite>    m_sprite;
    std::unique_ptr<olc::Decal>     m_decal;

public:
    TileLayer(nlohmann::json description, TinyEngine* engine);
    void LayerDraw(PixelGame* pixelRef);

};

class Level
{
private:
    std::vector<std::unique_ptr<Layer>> m_layers;
    std::vector<std::unique_ptr<Set>>   m_sets;
    nlohmann::json                      m_description;
    PixelGame*                          m_pixelGame;

    void AddLayer(nlohmann::json description, TinyEngine* engine);
    void AddSet(nlohmann::json description);

    template <typename T>
    void AddItem(nlohmann::json description, TinyEngine* engine);

public:
    Level(nlohmann::json description, PixelGame* pixelGame);
    bool Update(float fElapsedTime);
    void Draw();
    Set* getSet(int index);
};

class PixelGame : public olc::PixelGameEngine
{
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

class TinyEngine
{
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