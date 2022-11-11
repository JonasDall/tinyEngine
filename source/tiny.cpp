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

void Level::DisplayNames()
{
    for (int i{}; i < m_layers.size(); ++i)
    {
        m_layers[i].get()->DisplayLayerName();
    }
}

bool Level::LevelUpdate(float fElapsedTime){ return 1; }

void Level::LevelDraw(PixelGame* pixelRef)
{
    for (int i{}; i < m_layers.size(); ++i)
    {
        m_layers[i].get()->LayerDraw(pixelRef);
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// PixelGame
// --------------------------------------------------------------------------------------------------------------------------------------------
PixelGame::PixelGame(TinyEngine* engine) : m_engine{engine}
{
    // std::cout << "Bitch lasagna" << '\n';
    sAppName = engine->m_windowName;
}

PixelGame::~PixelGame()
{
    // std::cout << "Bitch banana" << '\n';
}

bool PixelGame::OnUserCreate()
{
    m_engine->LoadLevel(m_engine->GetLevelPath());
    return 1;
}

bool PixelGame::OnUserUpdate(float fElapsedTime)
{
    if (GetKey(olc::Key::ENTER).bPressed) return 0;

    // std::cout << "Update\n";
    
    // Clear(olc::WHITE);

    // m_engine->Update(fElapsedTime);
    m_engine->Draw();

    return 1;
}

bool PixelGame::OnUserDestroy(){ return 1; }

// --------------------------------------------------------------------------------------------------------------------------------------------
// TinyEngine
// --------------------------------------------------------------------------------------------------------------------------------------------
// Return reference to factory
// ComponentFactory& TinyEngine::Getfactory(){ return m_factory; }

// Load JSON level file
TinyEngine::TinyEngine(olc::vi2d resolution, std::string name, std::string level) : m_resolution{resolution}, m_windowName{name}, m_LevelPath{level}{}

TinyEngine::~TinyEngine(){}

void TinyEngine::LoadLevel(std::string path)
{
    std::ifstream file{ path };
    if (!file){ std::cout << "Could not open file\n"; return; }

    std::cout << "Map loaded\n";

    m_worldComponents.clear();

    nlohmann::json data = nlohmann::json::parse(file);

    std::cout << "Map parsed\n";

    for (int i{}; i < data.at("layers").size(); ++i)
    {
        // std::cout << data.at("layers")[i].at("type") << '\n';

        nlohmann::json description;
        description["description"] = data.at("layers")[i];

        m_currentLevel.AddLayer(description, this);
    }

    return;
}

Level* TinyEngine::GetLevel(){ return &m_currentLevel; }

void TinyEngine::AddComponentByID(nlohmann::json& description)
{
    // std::cout << description.at("id") << '\n';
    int id = description.at("id");
    nlohmann::json descCopy = description;
    m_worldComponents[id] = std::make_unique<Component>(descCopy);
}

bool TinyEngine::Update(float fElapsedTime)
{
    // m_currentLevel.Update(pixelRef);
    return 1;
}

void TinyEngine::Draw()
{
    m_pixelGameInstance.get()->Clear(olc::WHITE);
    // m_pixelGameInstance.get()->SetPixelMode(olc::Pixel::ALPHA);

    m_currentLevel.LevelDraw(m_pixelGameInstance.get());
    return;
}

void TinyEngine::Run()
{
    m_pixelGameInstance = std::make_unique<PixelGame>(this);
     if (m_pixelGameInstance.get()->Construct(m_resolution.x, m_resolution.y, 4, 4))
        m_pixelGameInstance.get()->Start();
    return;
}

std::string TinyEngine::GetLevelPath(){ return m_LevelPath; }

void TinyEngine::AddDefinition(std::string key, std::function<Component*()> lambda) { m_objectFactory[key] = lambda; }
std::function<Component*()> TinyEngine::GetDefinition(std::string key) {return m_objectFactory[key]; }
// --------------------------------------------------------------------------------------------------------------------------------------------
// ComponentFactory
// --------------------------------------------------------------------------------------------------------------------------------------------

}