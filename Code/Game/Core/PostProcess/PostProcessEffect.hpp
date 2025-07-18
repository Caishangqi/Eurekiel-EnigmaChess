#pragma once
#include <string>

struct RenderTarget;
class IRenderer;

/**
 * @class PostProcessEffect
 *
 * @brief An abstract base class for representing post-processing effects in a rendering pipeline.
 *
 * The PostProcessEffect class defines an interface for implementing post-processing effects
 * that operate on render targets. Each effect can be enabled or disabled and assigned a priority
 * for determining its execution order.
 *
 * Subclasses must override the Initialize, Shutdown, and Process methods to define the functionality
 * of the specific post-processing effect.
 */
class PostProcessEffect
{
public:
    PostProcessEffect(const std::string& name, int priority = 0);
    virtual ~PostProcessEffect() = default;

    // Initialize Resource
    /**
     * @brief Initializes the resources required for the post-processing effect.
     *
     * This method must be implemented by derived classes to set up any necessary
     * resources or configurations using the provided renderer instance. It is
     * called once before the post-processing effect starts processing.
     *
     * @param renderer A reference to the renderer instance used to initialize
     *        resources for the post-processing effect.
     */
    virtual void Initialize(IRenderer& renderer) = 0;
    virtual void Shutdown() = 0;

    // Process Effect
    /**
     * @brief Processes a post-processing effect on the given input render target and outputs the result to the specified output render target.
     *
     * This method should be implemented by subclasses to define the specific functionality of the post-processing effect.
     *
     * @param input The input render target on which the effect will be applied.
     * @param output The output render target where the processed result will be stored.
     */
    virtual void Process(RenderTarget* input, RenderTarget* output) = 0;

    // Enable and Disable
    void SetEnable(bool enabled) { m_enabled = enabled; }
    bool GetEnable() const { return m_enabled; }

    // Priority (the lower the number, the higher the priority)
    int  GetPriority() const { return m_priority; }
    void SetPriority(int priority) { m_priority = priority; }

    const std::string& GetName() const { return m_name; }

protected:
    std::string m_name     = "";
    int         m_priority = 0;
    bool        m_enabled  = false;
};
