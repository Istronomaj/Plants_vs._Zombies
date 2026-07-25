#include "pvz/app/HudRenderer.hpp"

#include "pvz/app/SpriteTable.hpp"

#include <array>
#include <string>

namespace pvz::app {
namespace {

constexpr std::array<PlantType, kPlantTypeCount> kPacketOrder{
    PlantType::Peashooter, PlantType::Wallnut, PlantType::Sunflower};

constexpr sf::Color kPanelFill{20, 24, 20, 190};
constexpr sf::Color kSelectedOutline{255, 214, 64};
constexpr sf::Color kAffordableOutline{235, 235, 235, 140};
constexpr sf::Color kUnaffordableTint{110, 110, 110, 160};

[[nodiscard]] std::string packetLabel(PlantType type, const GameConfig& config) {
    return std::to_string(config.plant(type).cost);
}

} // namespace

HudRenderer::HudRenderer(const ResourceManager& resources, sf::Vector2u windowSize)
    : m_resources(&resources),
      m_windowSize(windowSize),
      // sf::Text has no default constructor in SFML 3; the font is required here.
      m_sunText(resources.font(), "", 48),
      m_healthText(resources.font(), "", 48),
      m_waveText(resources.font(), "", 36),
      m_packetCostText(resources.font(), "", 28),
      m_packetIcon(resources.texture(TextureId::Sun)) {
    const auto width = static_cast<float>(windowSize.x);
    const auto height = static_cast<float>(windowSize.y);

    const float margin = width * 0.012F;
    const auto baseSize = static_cast<unsigned int>(height * 0.075F);

    m_sunText.setCharacterSize(baseSize);
    m_sunText.setFillColor(sf::Color{255, 214, 64});
    m_sunText.setOutlineColor(sf::Color::Black);
    m_sunText.setOutlineThickness(2.0F);
    m_sunText.setPosition({margin, margin});

    m_healthText.setCharacterSize(baseSize);
    m_healthText.setFillColor(sf::Color{235, 80, 80});
    m_healthText.setOutlineColor(sf::Color::Black);
    m_healthText.setOutlineThickness(2.0F);
    m_healthText.setPosition({margin, margin + static_cast<float>(baseSize) * 1.15F});

    m_waveText.setCharacterSize(static_cast<unsigned int>(height * 0.05F));
    m_waveText.setFillColor(sf::Color{225, 225, 245});
    m_waveText.setOutlineColor(sf::Color::Black);
    m_waveText.setOutlineThickness(2.0F);
    m_waveText.setPosition({margin, margin + static_cast<float>(baseSize) * 2.3F});

    m_packetSize = height * 0.13F;
    m_packetGap = m_packetSize * 0.16F;
    m_packetOriginX = margin;
    m_packetOriginY = height - m_packetSize - margin;

    m_packetBackground.setSize({m_packetSize, m_packetSize});
    m_packetCostText.setCharacterSize(static_cast<unsigned int>(m_packetSize * 0.26F));
}

Rect HudRenderer::packetBounds(PlantType type) const noexcept {
    for (std::size_t i = 0; i < kPacketOrder.size(); ++i) {
        if (kPacketOrder[i] == type) {
            const float x = m_packetOriginX + static_cast<float>(i) * (m_packetSize + m_packetGap);
            return Rect{{x, m_packetOriginY}, {m_packetSize, m_packetSize}};
        }
    }
    return {};
}

void HudRenderer::drawPacket(sf::RenderTarget& target, const World& world, PlantType type) {
    const Rect bounds = packetBounds(type);
    const int cost = world.config().plant(type).cost;
    const bool affordable = world.sun() >= cost;
    const bool selected = world.selectedPlant() == type;

    m_packetBackground.setPosition({bounds.position.x, bounds.position.y});
    m_packetBackground.setFillColor(kPanelFill);
    m_packetBackground.setOutlineThickness(selected ? 4.0F : 2.0F);
    m_packetBackground.setOutlineColor(selected ? kSelectedOutline : kAffordableOutline);
    target.draw(m_packetBackground);

    const SpriteSpec spec = spriteSpecFor(kindOf(type));
    const sf::Texture& texture = m_resources->texture(spec.texture);
    const auto textureSize = sf::Vector2f{texture.getSize()};
    if (textureSize.x > 0.0F && textureSize.y > 0.0F) {
        m_packetIcon.setTexture(texture, /*resetRect=*/true);
        const float fit =
            (m_packetSize * 0.62F) / std::max(textureSize.x, textureSize.y);
        m_packetIcon.setScale({fit, fit});
        m_packetIcon.setOrigin(textureSize / 2.0F);
        m_packetIcon.setPosition(
            {bounds.position.x + m_packetSize / 2.0F, bounds.position.y + m_packetSize * 0.42F});
        // Greyed out when the player cannot afford it, so the economy is
        // readable at a glance rather than being a line of text.
        m_packetIcon.setColor(affordable ? sf::Color::White : kUnaffordableTint);
        target.draw(m_packetIcon);
    }

    m_packetCostText.setString(packetLabel(type, world.config()));
    m_packetCostText.setFillColor(affordable ? sf::Color{255, 214, 64} : sf::Color{150, 150, 150});
    m_packetCostText.setOutlineColor(sf::Color::Black);
    m_packetCostText.setOutlineThickness(2.0F);

    const sf::FloatRect textBounds = m_packetCostText.getLocalBounds();
    m_packetCostText.setPosition(
        {bounds.position.x + (m_packetSize - textBounds.size.x) / 2.0F - textBounds.position.x,
         bounds.position.y + m_packetSize * 0.72F});
    target.draw(m_packetCostText);
}

void HudRenderer::draw(sf::RenderTarget& target, const World& world) {
    m_sunText.setString("Sun  " + std::to_string(world.sun()));
    m_healthText.setString("HP   " + std::to_string(world.playerHealth()));
    m_waveText.setString("Zombies left  " + std::to_string(world.zombiesRemaining()));

    target.draw(m_sunText);
    target.draw(m_healthText);
    target.draw(m_waveText);

    for (const PlantType type : kPacketOrder) {
        drawPacket(target, world, type);
    }
}

} // namespace pvz::app
