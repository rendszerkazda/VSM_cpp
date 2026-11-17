#pragma once

/**
 * @file PathProvider.h
 * @brief Where the Valheim save files live (Strategy pattern).
 */

#include <filesystem>
#include <memory>
#include <string>

namespace vsm {

/**
 * @brief Interface that answers "which folder holds the Valheim worlds?".
 */
class IPathProvider {
public:
    virtual ~IPathProvider() = default;

    /**
     * @brief Returns the directory that contains the .db and .fwl files.
     * @throws std::runtime_error if the directory cannot be determined.
     */
    virtual std::filesystem::path saveDir() const = 0;
};

/**
 * @brief The default Windows location of the Valheim worlds.
 *
 * `%USERPROFILE%\AppData\LocalLow\IronGate\Valheim\worlds_local`
 */
class WindowsPathProvider : public IPathProvider {
public:
    /**
     * @brief Creates the provider.
     * @param userProfile Value of the USERPROFILE variable. If empty, the
     *        real environment variable is read.
     */
    explicit WindowsPathProvider(std::string userProfile = "");

    /** @copydoc IPathProvider::saveDir */
    std::filesystem::path saveDir() const override;

private:
    /** @brief The user profile directory. */
    std::string userProfile_;
};

/**
 * @brief A provider that always returns a fixed directory.
 *
 * Used when the user gives --save-dir / "save_dir", and in tests.
 */
class FixedPathProvider : public IPathProvider {
public:
    /** @brief Creates the provider for the given directory. */
    explicit FixedPathProvider(std::filesystem::path dir);

    /** @copydoc IPathProvider::saveDir */
    std::filesystem::path saveDir() const override;

private:
    /** @brief The directory returned by saveDir(). */
    std::filesystem::path dir_;
};

/**
 * @brief Factory that picks the right provider.
 *
 * @param overrideDir If not empty, a FixedPathProvider for this directory is
 *        returned; otherwise the WindowsPathProvider.
 * @return The chosen provider.
 */
std::unique_ptr<IPathProvider> makePathProvider(const std::string& overrideDir);

}  // namespace vsm
