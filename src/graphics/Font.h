#pragma once

#include "../Globals.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "../storage/Reader.h"


namespace MINTGGGameEngine
{


/**
 * Represents a simple glyph-based font.
 *
 * Fonts are internally stored in the FONTX2 format (see
 * https://elm-chan.org/docs/dosv/fontx_e.html). They can be loaded from memory
 * or from a file.
 *
 * Glyphs are looked up by a 16-bit codepoint. This is usually taken to be a
 * Unicode code point in the Basic Multilingual Plane (BMP). Higher code points
 * are not currently supported.
 */
class Font
{
public:
    /**
     * \brief Load a font from the given FONTX2 buffer.
     *
     * Note that the buffer is NOT copied, but a direct reference to it is stored.
     * For this reason, the buffer must remain valid throughout the lifetime of
     * the Font object.
     *
     * @param rawData Pointer to the raw FONTX2 data.
     * @param fullSize Size of the FONTX2 buffer, in bytes.
     * @param ok Output parameter. Set to true if loading is successful, set to
     *      false otherwise. May be null.
     * @param outErrmsg Output parameter. Set to point to an error message if
     *      loading failed. May be null.
     * @return The loaded Font, or the default Font if loading failed.
     */
    static Font loadFONTX2Static (
        const uint8_t* rawData, size_t fullSize,
        bool* ok = nullptr, const char** outErrmsg = nullptr
        );

    /**
     * \brief Load a font from the given FONTX2 file.
     *
     * @param path Path to the FONTX2 file.
     * @param ok Output parameter. Set to true if loading is successful, set to
     *      false otherwise. May be null.
     * @param outErrmsg Output parameter. Set to point to an error message if
     *      loading failed. May be null.
     * @return The loaded Font, or the default Font if loading failed.
     */
    static Font loadFONTX2File (
        const std::string_view& path,
        bool* ok = nullptr, const char** outErrmsg = nullptr
        );

    /**
     * \brief Set the default font.
     *
     * This sets the font that is used when creating a Font object with the
     * default constructor.
     *
     * @param font The default font.
     */
    static void setDefaultFont(const Font& font);

    /**
     * \brief Register the given font.
     *
     * This adds the font to a font registry, from which it can be looked up by name.
     *
     * @param font The font to register.
     */
    static void registerFont(const Font& font);

    /**
     * \brief Check whether a font with the given name is registered.
     *
     * Fonts are only registered by calling registerFont().
     *
     * @param name The font name.
     * @return
     */
    static bool isFontRegistered(const std::string_view& name);

    /**
     * \brief Load the default fonts included with the engine.
     */
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
        ~Data();

        const uint8_t* rawData;
        bool bufOwned;

        std::string name;

        std::map<uint8_t, uint8_t*> explodedRawData;
    };

public:
    /**
     * \brief Create a font that always refers to the current default font.
     */
    Font();

    /**
     * \brief Create a font by name.
     *
     * The font must have previously been registered by calling registerFont().
     *
     * @param name The font name.
     * @see registerFont()
     */
    Font(const std::string_view& name);

    /**
     * \brief Copy constructor.
     *
     * This class is implicitly shared, so copying is cheap.
     *
     * @param other
     */
    Font(const Font& other) = default;


    /**
     * \brief Return the font name.
     *
     * A font name is stored in the FONTX2 file, but note that it is often not
     * useful. It might be a better idea to set a custom name using setName().
     *
     * @return The font name.
     */
    std::string getName() const;

    /**
     * \brief Set the font name.
     *
     * @param name Font name.
     */
    void setName(const std::string_view& name);

    /**
     * \brief Return the width of a single glyph in pixels.
     *
     * @return Glyph width in pixels.
     */
    uint8_t getGlyphWidth() const;

    /**
     * \brief Return the height of a single glyph in pixels.
     *
     * @return Glyph height in pixels.
     */
    uint8_t getGlyphHeight() const;

    /**
     * \brief Return the size of a single glyph in bytes.
     *
     * This is NOT the size in pixels!
     *
     * @return Glyph size in bytes.
     */
    size_t getGlyphSize() const;

    size_t getExplodedGlyphSize(uint8_t explosion) const;

    /**
     * \brief Return a pointer to the glyph data for the given codepoint.
     *
     * @param cp Codepoint (usually Unicode).
     * @return Data buffer.
     */
    const uint8_t* getGlyphBuffer(uint16_t cp) const;

    const uint8_t* getExplodedGlyphBuffer(uint16_t cp, uint8_t explosion) const;

    bool isValid() const;

    operator bool() const { return isValid(); }

    bool loadExplodedVersion(uint8_t explosion);

private:
    Font(nullptr_t) : d(nullptr) {}
    Font(const uint8_t* rawData, bool bufOwned);

    uint8_t* deriveExplodedFont(uint8_t delta) const;
    void explodeGlyph(uint8_t* exploded, const uint8_t* orig, uint8_t origWidth, uint8_t origHeight, uint8_t delta) const;

private:
    std::shared_ptr<Data> d;

    static Font defaultFont;
    static std::unordered_map<std::string, Font> fontRegistry;
};


}