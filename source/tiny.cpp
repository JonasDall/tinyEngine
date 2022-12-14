#include <iostream>
#include <fstream>
#include <functional>
#include <iostream>

#include "tiny.hpp"
#include "../lib/json.hpp"
#include "../lib/olcPGE.hpp"

namespace tiny
{
// --------------------------------------------------------------------------------------------------------------------------------------------
// int2d
// --------------------------------------------------------------------------------------------------------------------------------------------
int2d operator + (const int2d a, const int2d b)
{
    return int2d{a.x + b.x, a.y + b.y};
}

int2d operator - (const int2d a, const int2d b)
{
    return int2d{a.x - b.x, a.y - b.y};
}

int2d operator + (const int2d a, const float2d b)
{
    return int2d{ a.x + (int)b.x, a.y + (int)b.y };
}

int2d operator - (const int2d a, const float2d b)
{
    return int2d{ a.x - (int)b.x, a.y - (int)b.y };
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// float2d
// --------------------------------------------------------------------------------------------------------------------------------------------
float2d operator + (const float2d a, const float2d b)
{
    return float2d{a.x + b.x, a.y + b.y };
}

float2d operator - (const float2d a, const float2d b)
{
    return float2d{a.x - b.x, a.y - b.y };
}

float2d operator + (const float2d a, const int2d b)
{
    return float2d{a.x - (float)b.x, a.y - (float)b.y };
}

float2d operator - (const float2d a, const int2d b)
{
    return float2d{a.x - (float)b.x, a.y - (float)b.y };
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// Set
// --------------------------------------------------------------------------------------------------------------------------------------------
Set::Set(int gid, int col, int2d im, int2d ti, std::string path) : firstgid{gid}, columns{col}, imageRes{im}, tileRes{ti}{
    sprite = std::make_unique<olc::Sprite>(path);

    decal = std::make_unique<olc::Decal>(sprite.get());
}

int Set::getFirstgid(){ return firstgid; }
int Set::getColumns(){ return columns; }
int2d Set::getImageRes(){ return imageRes; }
int2d Set::getTileRes(){ return tileRes; }
olc::Sprite* Set::getSprite(){ return sprite.get(); }
olc::Decal*  Set::getDecal(){ return decal.get(); }

// --------------------------------------------------------------------------------------------------------------------------------------------
// Component
// --------------------------------------------------------------------------------------------------------------------------------------------
Component::Component(nlohmann::json description) : m_description{description}
{
    // std::cout << "Component with id " << description.at("id") << " created\n" << "Description: ";
    // std::cout << description << "\n\n";
}

bool Component::ComponentUpdate(float fElapsedTime){ return 1; }

void Component::ComponentDraw(PixelGame& pixelRef){}

bool Component::Signal(bool state) { return state; }

// Component::~Component() { std::cout << "Destruct" << '\n'; }

// --------------------------------------------------------------------------------------------------------------------------------------------
// Layer
// --------------------------------------------------------------------------------------------------------------------------------------------
Layer::Layer(nlohmann::json description)
{
    /*
    m_name = description.at("description").at("name");
    m_id = description.at("description").at("id");
    */
}

void Layer::DisplayLayerName()
{
    std::cout << m_name << '\n';
}

bool Layer::LayerUpdate(float fElapsedTime)
{
    return 1;
}

void Layer::LayerDraw(PixelGame* pixelRef){}

// --------------------------------------------------------------------------------------------------------------------------------------------
// ObjectLayer
// --------------------------------------------------------------------------------------------------------------------------------------------
ObjectLayer::ObjectLayer(nlohmann::json description) : Layer(description)
{
    // std::cout << description.at("description").at("objects").size() << '\n';
    // std::cout << description << "\n\n";

    /*
    for (int i{}; i < description.at("description").at("objects").size(); ++i)
    {
        nlohmann::json tempDescription = description.at("description").at("objects")[i];
    }
    */
}

void ObjectLayer::LayerDraw(PixelGame* pixelRef)
{
    // std::cout << "Drawing object\n";
    return;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// TileLayer
// --------------------------------------------------------------------------------------------------------------------------------------------
TileLayer::TileLayer(nlohmann::json description) : Layer(description)
{
    std::cout << "TileLayer created:\n" << description << '\n';
}

void TileLayer::LayerDraw(PixelGame* pixelRef){}

// --------------------------------------------------------------------------------------------------------------------------------------------
// Level
// --------------------------------------------------------------------------------------------------------------------------------------------
Level::Level(nlohmann::json description, PixelGame* pixelGame) : m_description{description}, m_pixelGame{pixelGame}
{
    // std::cout << description.at("tilesets") << '\n';

    for (int i{}; i < description.at("tilesets").size(); ++i)
        AddSet(description.at("tilesets")[i]);

    std::cout << "Added " << m_sets.size() << " tilesets\n";

    for (int i{}; i < description.at("layers").size(); ++i)
    {
        // std::cout << "Creating layer " << i << '\n';
        // std::cout << description.at("layers")[i] << '\n';
        AddLayer(description.at("layers")[i]);
    }
}

template <typename T>
void Level::AddItem(nlohmann::json description)
{
    // m_layers.push_back(std::make_unique<T>());
    m_layers.emplace_back(std::make_unique<T>(description));
}

void Level::AddLayer(nlohmann::json description)
{
    // std::cout << "Adding layer\n";
    // std::cout << description << '\n';
    std::string type = description.at("type");
    char typeKey = type[0];

    // std::cout << "Got type " << typeKey << '\n';
    switch(typeKey)
    {
        case 't':
        // m_layers.push_back(new TileLayer(description));
        AddItem<TileLayer>(description);
        break;

        case 'o':
        // m_layers.push_back( new ObjectLayer(description));
        AddItem<ObjectLayer>(description);
        break;
    }
}

void Level::AddSet(nlohmann::json description)
{
    std::cout << description.at("firstgid") << '\n';
    m_gids.push_back( description.at("firstgid") );
    // int firstgid = description.at("firstgid");
    std::string relPath = description.at("source");
    // std::cout << relPath << '\n';
    std::string fullPath = "asset/tiled/";
    // std::cout << fullPath << '\n';

    std::ifstream file{ fullPath + relPath };
    if(!file.is_open()){
    std::cout << "File does not exist\n";
    return;
    }

    nlohmann::json setData = nlohmann::json::parse(file);

    // int columns{ setData.at("columns") };
    int2d imageRes{ setData.at("imagewidth"), setData.at("imageheight") };
    int2d tileRes{ setData.at("tilewidth"), setData.at("tileheight") };

    std::string imagePath = fullPath;
    imagePath += setData.at("image");

    m_sets.push_back(std::make_unique<Set>(description.at("firstgid"), setData.at("columns"), imageRes, tileRes, imagePath));
}

bool Level::Update(float fElapsedTime){ return 1; }

void Level::Draw()
{
    for (int i{}; i < m_layers.size(); ++i)
    {
        m_layers[i].get()->LayerDraw(m_pixelGame);
    }
}

int Level::getSetIndex(int gid)
{
    std::cout << "testing " << gid << " against \n";

    for (int i{}; i < m_gids.size(); ++i)
    {
        std::cout << m_gids[i] << '\n';

        if (gid < m_gids[i])
        {   
            std::cout << m_gids[i-1] << " < " << gid << " < " <<m_gids[i] << '\n';
            return i - 1;
        }
    }

    return 0;
}

Set* Level::getSet(int index){

    if (index > m_sets.size())
    return m_sets[index].get();

    return nullptr;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// PixelGame
// --------------------------------------------------------------------------------------------------------------------------------------------
PixelGame::PixelGame(TinyEngine* engine, std::string level) : m_engine{engine}, m_firstLevel{level}
{
    // std::cout << "Bitch lasagna" << '\n';
    sAppName = engine->GetName();
}

PixelGame::~PixelGame()
{
    // std::cout << "Bitch banana" << '\n';
}

bool PixelGame::OnUserCreate()
{
    m_engine->LoadLevel(m_firstLevel);
    return 1;
}

bool PixelGame::OnUserUpdate(float fElapsedTime)
{
    if (GetKey(olc::Key::ENTER).bPressed) return 0;

    // m_engine->GetLevel()->Update(fElapsedTime);
    m_engine->GetLevel()->Draw();

    return 1;
}

bool PixelGame::OnUserDestroy(){ return 1; }

// --------------------------------------------------------------------------------------------------------------------------------------------
// TinyEngine
// --------------------------------------------------------------------------------------------------------------------------------------------
// Return reference to factory
// ComponentFactory& TinyEngine::Getfactory(){ return m_factory; }

// Load JSON level file
TinyEngine::TinyEngine(olc::vi2d resolution, std::string name) : m_resolution{resolution}, m_name{name}{}

TinyEngine::~TinyEngine(){}

void TinyEngine::LoadLevel(std::string path)
{
    std::ifstream file{ path };
    if (!file){ std::cout << "Could not open file\n"; return; }

    std::cout << "Map loaded\n";

    nlohmann::json data = nlohmann::json::parse(file);

    m_level = std::make_unique<Level>(data, m_pixelGame.get());

    std::cout << "Map parsed\n";

    return;
}

Level* TinyEngine::GetLevel(){
    return m_level.get();
    }

void TinyEngine::AddComponentByID(nlohmann::json& description)
{
    // std::cout << description.at("id") << '\n';
    int id = description.at("id");
    nlohmann::json descCopy = description;
    // m_worldComponents[id] = std::make_unique<Component>(descCopy);
}

void TinyEngine::Run(std::string level)
{
    m_pixelGame = std::make_unique<PixelGame>(this, level);
     if (m_pixelGame.get()->Construct(m_resolution.x, m_resolution.y, 4, 4))
        m_pixelGame.get()->Start();
    return;
}

void TinyEngine::AddDefinition(std::string key, std::function<Component*()> lambda) { m_objectFactory[key] = lambda; }

std::string TinyEngine::GetName() { return m_name; }

std::function<Component*()> TinyEngine::GetDefinition(std::string key) {return m_objectFactory[key]; }
// --------------------------------------------------------------------------------------------------------------------------------------------
// ComponentFactory
// --------------------------------------------------------------------------------------------------------------------------------------------

}