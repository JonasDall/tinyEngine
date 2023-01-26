#include <iostream>
#include <fstream>
#include <functional>
#include <string>

#include "tiny.hpp"
#include "../lib/json.hpp"
#include "../lib/olcPGE.hpp"

namespace tiny
{
// --------------------------------------------------------------------------------------------------------------------------------------------
// Functions
// --------------------------------------------------------------------------------------------------------------------------------------------
olc::Pixel hexToPixel(std::string hex)
{
    hex.erase(hex.begin());
    unsigned int intValues[4]{};

    if (hex.length() == 6)
    {
        hex = "ff" + hex;
    }

    if (hex.length() != 8)  return olc::WHITE;

    for (int i{}; i < hex.length() / 2; ++i)
    {
        std::string temp{"0x"};
        temp.push_back(hex.at(i * 2));
        temp.push_back(hex.at((i * 2)+ 1));

        intValues[i] = std::stoul(temp, nullptr, 16);
    }

    return {static_cast<uint8_t>(intValues[1]), static_cast<uint8_t>(intValues[2]), static_cast<uint8_t>(intValues[3]), static_cast<uint8_t>(intValues[0])};
}

// -------------------------------------------------------
// Set
// --------------------------------------------------------------------------------------------------------------------------------------------
Set::Set(int gid, int col, olc::vi2d im, olc::vi2d ti, std::string path) : firstgid{gid}, columns{col}, imageRes{im}, tileRes{ti}{
    sprite = std::make_unique<olc::Sprite>(path);

    decal = std::make_unique<olc::Decal>(sprite.get());
}

int Set::getFirstgid(){ return firstgid; }
int Set::getColumns(){ return columns; }
olc::vi2d Set::getImageRes(){ return imageRes; }
olc::vi2d Set::getTileRes(){ return tileRes; }
olc::Sprite* Set::getSprite(){ return sprite.get(); }
olc::Decal*  Set::getDecal(){ return decal.get(); }

// --------------------------------------------------------------------------------------------------------------------------------------------
// Component
// --------------------------------------------------------------------------------------------------------------------------------------------
Component::Component(nlohmann::json description)
{
    // std::cout << "Component with id " << description.at("id") << " created\n" << "Description: ";
    // std::cout << description << "\n\n";
}

bool Component::ComponentUpdate(float fElapsedTime){ return 1; }

void Component::ComponentDraw(PixelGame& pixelRef){}

// Component::~Component() { std::cout << "Destruct" << '\n'; }

// --------------------------------------------------------------------------------------------------------------------------------------------
// Layer
// --------------------------------------------------------------------------------------------------------------------------------------------
Layer::Layer(nlohmann::json description, TinyEngine* engine, PixelGame* game, Level* level) : m_engine{engine}, m_game{game}, m_level{level}
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

void Layer::LayerDraw(){}

// --------------------------------------------------------------------------------------------------------------------------------------------
// ObjectLayer
// --------------------------------------------------------------------------------------------------------------------------------------------
ObjectLayer::ObjectLayer(nlohmann::json description, TinyEngine* engine, PixelGame* game, Level* level) : Layer(description, engine, game, level)
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

void ObjectLayer::LayerDraw()
{
    // std::cout << "Drawing object\n";
    return;
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// TileLayer
// --------------------------------------------------------------------------------------------------------------------------------------------
TileLayer::TileLayer(nlohmann::json description, TinyEngine* engine, PixelGame* game, Level* level) : Layer(description, engine, game, level)
{
    std::cout << "TileLayer created:\n";

    m_height = description.at("height");
    m_width = description.at("width");

    for (int i{}; i < m_height; ++i)
    {
        std::vector<int> tempRow;

        for (int j{}; j < m_width; ++j)
        {
            int index = (m_width * i) + j;

            if ( index < description.at("data").size() )
                tempRow.push_back( description.at("data")[index] );
        }

        m_mapData.push_back(tempRow);
    }

    if (description.contains("tintcolor"))
    {   
        m_tint = hexToPixel(description.at("tintcolor"));
    }

    /*
    if (engine == nullptr || game == nullptr || level == nullptr)
        std::cout << "Not working!\n";
    else
        std::cout << "Working as it should\n";

    for (int i{}; i < m_mapData.size(); ++i)
        std::cout << m_mapData[i];    
    std::cout << '\n';
    */
}

void TileLayer::LayerDraw()
{
    //Column
    for (int column{}; column < m_height; ++column)
    {
        //Row
        for (int row{}; row < m_width; ++row)
        {
            int tileData = m_mapData[column][row];

            if (tileData != 0)
            {
                int setIndex = m_level->getSetIndex(tileData);
                Set* set = m_level->getSet(setIndex);

                int relTileData = tileData - set->getFirstgid() + 1;

                olc::vi2d texturePos{};
                texturePos.x = (relTileData - 1) % set->getColumns();
                texturePos.y = (relTileData - 1) / set->getColumns();

                m_game->DrawPartialDecal(
                    {(float)(row * 16), (float)(column * 16)},                                                                              //Screen position
                    {(float)set->getTileRes().x, (float)set->getTileRes().y},                                                               //Size on screen
                    set->getDecal(),                                                                                                        //Decal
                    {(float)(texturePos.x * set->getTileRes().x) + (float)0.01, (float)(texturePos.y * set->getTileRes().y) + (float)0.01}, //Texture position
                    {(float)set->getTileRes().x - (float)0.02, (float)set->getTileRes().y - (float)0.02},                                   //Size on texture
                    m_tint                                                                                                                  //Tint
                    );
            }
        }
        // std::cout << '\n';
    }

    // std::cout << '\n';
    // m_game->DrawDecal({0, 0}, m_level->getSet(0)->getDecal());
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// Level
// --------------------------------------------------------------------------------------------------------------------------------------------
Level::Level(nlohmann::json description, TinyEngine* engine, PixelGame* pixelGame) : m_description{description}, m_engine{engine}, m_pixelGame{pixelGame}
{
    // std::cout << description.at("tilesets") << '\n';

    for (int i{}; i < description.at("tilesets").size(); ++i)
        AddSet(description.at("tilesets")[i]);

    // std::cout << "Added " << m_sets.size() << " tilesets\n";

    for (int i{}; i < description.at("layers").size(); ++i)
    {
        // std::cout << "Creating layer " << i << '\n';
        // std::cout << description.at("layers")[i] << '\n';
        AddLayer(description.at("layers")[i]);
    }
}

template <typename T>
void Level::AddItem(nlohmann::json description){
    // m_layers.push_back(std::make_unique<T>());
    m_layers.emplace_back(std::make_unique<T>(description, m_engine, m_pixelGame, this));
}

void Level::AddLayer(nlohmann::json description){
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

void Level::AddSet(nlohmann::json description){
    // std::cout << description.at("firstgid") << '\n';
    // m_gids.push_back( description.at("firstgid") );
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
    olc::vi2d imageRes{ setData.at("imagewidth"), setData.at("imageheight") };
    olc::vi2d tileRes{ setData.at("tilewidth"), setData.at("tileheight") };

    std::string imagePath = fullPath;
    imagePath += setData.at("image");

    m_sets.push_back(std::make_unique<Set>(description.at("firstgid"), setData.at("columns"), imageRes, tileRes, imagePath));
}

bool Level::Update(float fElapsedTime){ return 1; }

void Level::Draw(){
    for (int i{}; i < m_layers.size(); ++i)
    {
        m_layers[i].get()->LayerDraw();
    }
}

int Level::getSetIndex(int tile){

    for (int i{}; i < m_sets.size(); ++i)
    {
        if (i + 1 == m_sets.size())
            return i;
        
        if (tile < m_sets[i + 1].get()->getFirstgid())
            return i;
    }
    return 0;
}

Set* Level::getSet(int index){

    // std::cout << "Index " << index << " is within " << m_sets.size() << '\n';

    if (index < m_sets.size())
    {
        return m_sets[index].get();
    }

    return nullptr;
}

void Level::printSets(){
    for (int i{}; i < m_sets.size(); ++i)
    {
        std::cout << "Tileset " << i << " with firstgid " << m_sets[i].get()->getFirstgid() << '\n';
    }
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
    Clear(olc::GREY);

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

    m_level = std::make_unique<Level>(data, this, m_pixelGame.get());

    std::cout << "Map parsed\n";

    return;
}

Level* TinyEngine::GetLevel(){ return m_level.get(); }

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