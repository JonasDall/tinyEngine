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
Layer::Layer(nlohmann::json description, TinyEngine* engine) : m_engine{engine}
{
    m_name = description.at("description").at("name");
    m_id = description.at("description").at("id");
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
ObjectLayer::ObjectLayer(nlohmann::json description, TinyEngine* engine) : Layer(description, engine)
{
    // std::cout << description.at("description").at("objects").size() << '\n';
    // std::cout << description << "\n\n";

    for (int i{}; i < description.at("description").at("objects").size(); ++i)
    {
        nlohmann::json tempDescription = description.at("description").at("objects")[i];

        engine->AddComponentByID(tempDescription);
    }
}

void ObjectLayer::LayerDraw(PixelGame* pixelRef)
{
    // std::cout << "Drawing object\n";
    return;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// TileLayer
// --------------------------------------------------------------------------------------------------------------------------------------------
TileLayer::TileLayer(nlohmann::json description, TinyEngine* engine) : Layer(description, engine)
{


    std::cout << description.at("description").at("data") << '\n';

    for (int i{}; i < description.at("description").at("data").size(); ++i)
    {
        m_mapData.push_back(description.at("description").at("data")[i]);
    }

    m_columns = description.at("description").at("height");
    m_rows = description.at("description").at("width");


    for (int i{}; i < description.at("description").at("properties").size(); ++i)
    {
        std::string prop = description.at("description").at("properties")[i].at("name");
        char propKey = prop[0];
        // std::cout << propKey << '\n';

        switch (propKey)
        {
            case 's':
            m_imagePath = description.at("description").at("properties")[i].at("value");
            std::cout << "Image: " << m_imagePath << '\n';
            m_sprite = std::make_unique<olc::Sprite>("asset/" + m_imagePath);
            m_decal = std::make_unique<olc::Decal>(m_sprite.get());
            break;

            case 't':
            // std::cout << description.at("description").at("properties");
            m_tileSize = description.at("description").at("properties")[i].at("value");
            std::cout << "Tilesize: " << m_tileSize << '\n';
            break;
        }
    }

    m_textureColumns = m_sprite.get()->height / m_tileSize;
    m_textureRows = m_sprite.get()->width / m_tileSize;
}

void TileLayer::LayerDraw(PixelGame* pixelRef)
{
    // std::cout << "Drawing tile\n";

    for (int column{}; column < m_columns; ++column)
    {
        for (int row{}; row < m_rows; ++row)
        {
            int tileToDraw = m_mapData[column * m_rows + row];

            if (tileToDraw != 0)
            {
                int tempColumn = (tileToDraw -1) / m_textureRows;
                int tempRow = tileToDraw - (tempColumn * m_textureRows);

                pixelRef->DrawPartialDecal(
                    {(float)(row * m_tileSize), (float)(column * m_tileSize)}, // position on screen
                    {(float)m_tileSize, (float)m_tileSize}, // size on screen
                    m_decal.get(), // decal
                    {(float)((tempRow -1) * m_tileSize) + 0.01f, (float)(tempColumn * m_tileSize) + 0.01f}, //position in texture
                    {(float)m_tileSize - 0.02f, (float)m_tileSize - 0.02f}); //size in texture
            }
        }
    }
    // pixelRef->DrawDecal(olc::vf2d(0, 0), m_decal.get());
    // pixelRef->DrawSprite(olc::vi2d(64, 0), m_sprite.get());
    return;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// Level
// --------------------------------------------------------------------------------------------------------------------------------------------
Level::Level(nlohmann::json description, PixelGame* pixelGame) : m_description{description}, m_pixelGame{pixelGame}
{
    // std::cout << description.at("tilesets") << '\n';

    for (int i{}; i < description.at("tilesets").size(); ++i)
        AddSet(description.at("tilesets")[i]);

    std::cout << "Added " << m_sets.size() << " tilesets\n";
}

template <typename T>
void Level::AddItem(nlohmann::json description, TinyEngine* engine)
{
    // m_layers.push_back(std::make_unique<T>());
    m_layers.emplace_back(std::make_unique<T>(description, engine));
}

void Level::AddLayer(nlohmann::json description, TinyEngine* engine)
{
    std::string type = description.at("description").at("type");
    char typeKey = type[0];

    switch(typeKey)
    {
        case 't':
        // m_layers.push_back(new TileLayer(description));
        AddItem<TileLayer>(description, engine);
        break;

        case 'o':
        // m_layers.push_back( new ObjectLayer(description));
        AddItem<ObjectLayer>(description, engine);
        break;
    }
}

void Level::AddSet(nlohmann::json description)
{
    // std::cout << description << '\n';
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