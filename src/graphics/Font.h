#pragma once

#include "../Globals.h"

#include <memory>
#include <string>
#include <unordered_map>

#include "../storage/Reader.h"


namespace MINTGGGameEngine
{


class Font
{
public:
    static Font loadFONTX2Static (
        const uint8_t* rawData, size_t fullSize,
        bool* ok = nullptr, const char** outErrmsg = nullptr
        );

    static Font loadFONTX2File (
        const std::string_view& path,
        bool* ok = nullptr, const char** outErrmsg = nullptr
        );

    static void setDefaultFont(const Font& font);

    static void registerFont(const Font& font);

    static bool isFontRegistered(const std::string_view& name);

    static void loadDefaultFonts();

private:
    static Font loadFONTX2 (
        const uint8_t* rawData, size_t fullSize,
        bool bufOwned, bool* ok,
        const char** outErrmsg
        );

private:
    struct Data
    {
        Data(const uint8_t* rawData, bool bufOwned) : rawData(rawData), bufOwned(bufOwned) {}
        ~Data() { if (bufOwned) free(const_cast<uint8_t*>(rawData)); }

        const uint8_t* rawData;
        bool bufOwned;

        std::string name;
    };

public:
    Font();
    Font(const std::string_view& name);
    Font(const Font& other) = default;

    std::string getName() const;
    void setName(const std::string_view& name);

    uint8_t getGlyphWidth() const;
    uint8_t getGlyphHeight() const;

    size_t getGlyphSize() const;

    const uint8_t* getGlyphBuffer(uint16_t cp) const;

private:
    Font(nullptr_t) : d(nullptr) {}
    Font(const uint8_t* rawData, bool bufOwned);

private:
    std::shared_ptr<Data> d;

    static Font defaultFont;
    static std::unordered_map<std::string, Font> fontRegistry;
};


}