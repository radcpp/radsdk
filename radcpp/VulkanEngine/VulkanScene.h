#ifndef VULKAN_SCENE_H
#define VULKAN_SCENE_H

#include "VulkanCore.h"
#include "VulkanCamera.h"
#include "radcpp/Common/Geometry.h"

struct VulkanLight
{
    enum class Type : uint32_t
    {
        Point,
        Directional,
        Spot,
        Area,
        Volume,
        Count,
    };
    glm::vec3   position;
    Type        type;
    glm::vec3   direction;
    float       range; // distance cutoff for point and spot lights
    glm::vec3   color;
    float       intensity;
    glm::vec2   shape;
    float       innerConeAngle;
    float       outerConeAngle;
}; // struct VulkanLight

class VulkanSceneNode;
class VulkanMesh;
class VulkanMaterial;

struct VulkanTexture;

class VulkanScene : public RefCounted<VulkanScene>
{
public:
    VulkanScene(Ref<VulkanDevice> device);
    ~VulkanScene();

    bool Import(const Path& filePath);
    BoundingBox GetBoundingBox() const;

    Ref<VulkanDevice> m_device;
    std::vector<Ref<VulkanMesh>> m_meshes;
    std::vector<Ref<VulkanMaterial>> m_materials;
    Ref<VulkanCamera> m_camera;
    std::vector<VulkanLight> m_lights;

    Ref<VulkanSceneNode> m_rootNode;

}; // class VulkanScene

class VulkanSceneNode : public RefCounted<VulkanSceneNode>
{
public:
    VulkanSceneNode(VulkanSceneNode* parent, std::string_view name);
    ~VulkanSceneNode();

    void AddChild(Ref<VulkanSceneNode> childNode);

    BoundingBox GetBoundingBox() const;

    std::string m_name;

    VulkanSceneNode* m_parent;
    std::vector<Ref<VulkanSceneNode>> m_children;

    glm::mat4 m_transform; // the transformation relative to the node's parent

    std::vector<Ref<VulkanMesh>> m_meshes;
}; // class VulkanSceneNode

class VulkanMesh : public RefCounted<VulkanMesh>
{
public:
    VulkanMesh(VulkanScene* scene, std::string_view name);
    ~VulkanMesh();

    uint32_t GetVertexCount() const { return m_vertexCount; }
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indices.size()); }

    VulkanScene* m_scene;
    std::string m_name;
    std::vector<uint32_t> m_indices;

    Ref<VulkanBuffer> m_vertexBuffer;
    Ref<VulkanBuffer> m_indexBuffer;

    uint32_t        m_vertexCount = 0;
    uint32_t        m_vertexStride;
    VkDeviceSize    m_vertexBufferOffset = 0;
    VkDeviceSize    m_vertexBufferSize = 0;
    VkDeviceSize    m_indexBufferOffset = 0;
    VkDeviceSize    m_indexBufferSize = 0;

    Ref<VulkanMaterial> m_material;

    bool m_hasPosition;
    bool m_hasNormal;
    bool m_hasTangent;
    bool m_hasUV;
    uint32_t m_numUVChannels;
    bool m_hasColor;

    Ref<VulkanDescriptorSet> m_descriptorSet;
    Ref<VulkanGraphicsPipeline> m_pipeline;

    BoundingBox m_aabb = {};

}; // class VulkanMesh

enum TextureMapping
{
    TextureMappingUV        = 0x0,
    TextureMappingSphere    = 0x1,
    TextureMappingCylinder  = 0x2,
    TextureMappingBox       = 0x3,
    TextureMappingPlane     = 0x4,
    TextureMappingOther     = 0x5,
};

// Defines how the Nth texture of a specific type is combined with the result of all previous layers.
enum TextureOp
{
    TextureOpMultiply = 0x0,    // T = T1 * T2
    TextureOpAdd = 0x1,         // T = T1 + T2
    TextureOpSubtract = 0x2,    // T = T1 - T2
    TextureOpDivide = 0x3,      // T = T1 / T2
    TextureOpSmoothAdd = 0x4,   // T = (T1 + T2) - (T1 * T2)
    TextureOpSignedAdd = 0x5,   // T = T1 + (T2-0.5)
};

struct VulkanTexture : public RefCounted<VulkanTexture>
{
    Path filePath;
    Ref<VulkanImage> image;
    TextureMapping mapping = TextureMappingUV;
    uint32_t texCoordIndex = 0;
    float blend = 0.0f;
    TextureOp op;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
}; // struct VulkanTexture

class VulkanMaterial : public RefCounted<VulkanMaterial>
{
public:
    VulkanMaterial(VulkanScene* scene, std::string_view name);
    ~VulkanMaterial();

    VulkanScene* m_scene;
    std::string m_name;
    Path m_baseDir;

    Ref<VulkanTexture> m_displacementTexture;
    Ref<VulkanTexture> m_normalTexture;

    glm::vec3 m_baseColor;
    float m_opacity = 1.0f;
    Ref<VulkanTexture> m_baseColorTexture;

    float m_metallic; // the metallic-ness (0 = dielectric, 1 = metallic).
    float m_roughness; // surface roughness, controls both diffuse and specular response.
    Ref<VulkanTexture> m_metallicRoughnessTexture;

    glm::vec3 m_emissiveColor;
    float m_emissiveIntensity = 1.0f;
    Ref<VulkanTexture> m_emissiveTexture;

    glm::vec3 m_ambientColor;
    float m_ambientWeight = 1.0f;
    Ref<VulkanTexture> m_ambientTexture;

}; // class VulkanMaterial

#endif // VULKAN_SCENE_H