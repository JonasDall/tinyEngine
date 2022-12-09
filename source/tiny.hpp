#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>

#include "../lib/olcPGE.hpp"
#include "../lib/json.hpp"

namespace tiny
{

struct int2d{
int x;
int y;
};

struct float2d{
float x;
float y;
};

struct tileSet{
int firstgid = 0;
std::string path = "";
int2d imageRes = {0, 0};
int2d tileRes = {0, 0};
olc::Decal image;
};

class Level;
class TinyEngine;
class PixelGame;
class Component;
// class ComponentFactory;

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
    std::vector<tileSet>                m_tileSets;

protected:
    template <typename T>
    void AddItem(nlohmann::json description, TinyEngine* engine);

public:
    void AddLayer(nlohmann::json description, TinyEngine* engine);
    void AddTileset(nlohmann::json description);
    void DisplayNames();
    bool LevelUpdate(float fElapsedTime);
    void LevelDraw(PixelGame* pixelRef);
};

class PixelGame : public olc::PixelGameEngine
{
private:
    TinyEngine* m_engine;

public:
    PixelGame(TinyEngine* engine);
    ~PixelGame();
    bool OnUserCreate();
    bool OnUserUpdate(float fElapsedTime);
    bool OnUserDestroy();
};

class TinyEngine
{
private:
    // std::map<int, std::unique_ptr<Component>>           m_worldComponents;

    std::map<std::string, std::unique_ptr<Level>>       m_levels;
    std::string                                         m_currentLevelName;
    // std::unique_ptr<ComponentFactory>                   m_factory;
    std::unique_ptr<PixelGame>                          m_pixelGameInstance;
    std::string                                         m_LevelPath;
    olc::vi2d                                           m_resolution;
    std::map<std::string, std::function<Component*()>>  m_objectFactory;
    bool                                                m_constructed{};

public:
    std::string                                         m_windowName;

    TinyEngine(olc::vi2d resolution, std::string name, std::string level);
    ~TinyEngine();

    // ComponentFactory& Getfactory();
    void LoadLevel(std::string path, std::string name);
    Level* GetLevel(std::string name);
    Component* GetComponentByID(int id);
    void AddComponentByID(nlohmann::json& description);
    bool Update(float fElapsedTIme);
    void Draw();
    void Run();
    std::string GetLevelPath();

    void AddDefinition(std::string key, std::function<Component*()> lambda);
    std::function<Component*()> GetDefinition(std::string key);
};

/*
class ComponentFactory
{
private:

public:
    ComponentFactory() = default;
    void AddDefinition(std::string key, std::function<Component*()> lambda);
    std::function<Component*()> GetDefinition(std::string key);

};
*/

}