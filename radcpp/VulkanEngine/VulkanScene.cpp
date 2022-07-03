#include "VulkanScene.h"

#include "assimp/scene.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"

glm::vec2 ToVec2(const aiVector2D& vec)
{
    return glm::vec2(vec[0], vec[1]);
}

glm::vec3 ToVec3(const aiVector3D& vec)
{
    return glm::vec3(vec[0], vec[1], vec[2]);
}

glm::vec3 ToVec3(const aiColor3D& vec)
{
    return glm::vec3(vec.r, vec.g, vec.b);
}

glm::vec4 ToVec4(const aiColor4D& vec)
{
    return glm::vec4(vec.r, vec.g, vec.b, vec.a);
}

glm::mat4 ToMat4(const aiMatrix4x4& matrix)
{
    return glm::mat4(
        matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0],
        matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1],
        matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2],
        matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]
    );
}

VulkanScene::VulkanScene(Ref<VulkanDevice> device) :
    m_device(device)
{
    m_rootNode = MakeRefCounted<VulkanSceneNode>(nullptr, "Root");
    m_camera = MakeRefCounted<VulkanCamera>();
}

VulkanScene::~VulkanScene()
{
}

class VulkanAsset : public RefCounted<VulkanAsset>
{
public:
    VulkanScene* m_scene;
    std::string m_fileName;
    Path m_baseDir;
    const aiScene* m_asset = nullptr;
    std::vector<Ref<VulkanMesh>> m_meshes;
    std::vector<Ref<VulkanMaterial>> m_materials;
    std::vector<VulkanLight> m_lights;
    Ref<VulkanSceneNode> m_rootNode;
    std::map<Path, Ref<VulkanImage>> m_images;

    VulkanAsset(VulkanScene* scene) :
        m_scene(scene)
    {
    }
    ~VulkanAsset()
    {
        if (m_asset != nullptr)
        {
            aiReleaseImport(m_asset);
        }
    }

    bool Import(const Path& filePath)
    {
        m_fileName = (const char*)filePath.u8string().c_str();
        m_baseDir = std::filesystem::absolute(filePath).remove_filename();
        m_rootNode = MakeRefCounted<VulkanSceneNode>(m_scene->m_rootNode.get(), m_fileName);

        int processFlags =
            aiProcessPreset_TargetRealtime_Fast |
            aiProcess_FlipUVs |
            aiProcess_GenBoundingBoxes;
        m_asset = aiImportFile((const char*)filePath.u8string().c_str(),
            processFlags);
        if (!m_asset)
        {
            LogPrint("Vulkan", LogLevel::Error, "aiImportFile: failed to import '%s': %s",
                m_fileName.c_str(),
                aiGetErrorString());
            return false;
        }

        m_materials.resize(m_asset->mNumMaterials);
        for (uint32_t i = 0; i < m_asset->mNumMaterials; i++)
        {
            const aiMaterial* materialData = m_asset->mMaterials[i];
            m_materials[i] = MakeRefCounted<VulkanMaterial>(m_scene, materialData->GetName().C_Str());
            InitMaterial(m_materials[i].get(), materialData);
        }

        m_meshes.resize(m_asset->mNumMeshes);
        for (uint32_t i = 0; i < m_asset->mNumMeshes; i++)
        {
            const aiMesh* meshData = m_asset->mMeshes[i];
            m_meshes[i] = MakeRefCounted<VulkanMesh>(m_scene, meshData->mName.C_Str());
            InitMesh(m_meshes[i].get(), meshData);
        }

        m_lights.resize(m_asset->mNumLights);
        for (uint32_t i = 0; i < m_asset->mNumLights; i++)
        {
            VulkanLight& light = m_lights[i];
            aiLight* lightData = m_asset->mLights[i];
            light.position = ToVec3(lightData->mPosition);
            light.type = VulkanLight::Type::Point;
            if (lightData->mType == aiLightSource_DIRECTIONAL)
            {
                light.type = VulkanLight::Type::Directional;
            }
            if (lightData->mType == aiLightSource_POINT)
            {
                light.type = VulkanLight::Type::Point;
            }
            if (lightData->mType == aiLightSource_SPOT)
            {
                light.type = VulkanLight::Type::Spot;
            }
            if (lightData->mType == aiLightSource_AREA)
            {
                light.type = VulkanLight::Type::Area;
            }
            light.direction = ToVec3(lightData->mDirection);
            lightData->mAttenuationConstant;
            lightData->mAttenuationLinear;
            lightData->mAttenuationQuadratic;
            light.range = FLT_MAX; // distance cutoff for point and spot lights
            light.color = ToVec3(lightData->mColorDiffuse);
            light.intensity = 1.0f;
            light.shape = { lightData->mSize.x, lightData->mSize.y };
            light.innerConeAngle = lightData->mAngleInnerCone;
            light.outerConeAngle = lightData->mAngleOuterCone;
        }

        m_rootNode->m_name = m_asset->mRootNode->mName.C_Str();
        InitNodes(m_rootNode.get(), m_asset->mRootNode);
        return true;
    }

    bool InitMesh(VulkanMesh* mesh, const aiMesh* meshData)
    {
        mesh->m_vertexStride = 0;
        if (meshData->HasPositions())
        {
            mesh->m_vertexStride += sizeof(glm::vec3);
            mesh->m_hasPosition = true;
        }
        if (meshData->HasNormals())
        {
            mesh->m_vertexStride += sizeof(glm::vec3);
            mesh->m_hasNormal = true;
        }
        if (meshData->HasTangentsAndBitangents())
        {
            mesh->m_vertexStride += sizeof(glm::vec4);
            mesh->m_hasTangent = true;
        }
        uint32_t numUVChannels = meshData->GetNumUVChannels();
        if (numUVChannels > 0)
        {
            mesh->m_vertexStride += numUVChannels * sizeof(glm::vec2);
            mesh->m_hasUV = true;
            mesh->m_numUVChannels = numUVChannels;
        }
        if (uint32_t numColorChannels = meshData->GetNumColorChannels())
        {
            mesh->m_vertexStride += sizeof(glm::vec4);
            mesh->m_hasColor = true;
        }
        mesh->m_vertexBufferSize = VkDeviceSize(meshData->mNumVertices) * VkDeviceSize(mesh->m_vertexStride);
        mesh->m_indexBufferSize = VkDeviceSize(meshData->mNumFaces) * 3 * sizeof(uint32_t);

        mesh->m_vertexBuffer = m_scene->m_device->CreateVertexBuffer(mesh->m_vertexBufferSize);
        mesh->m_indexBuffer = m_scene->m_device->CreateIndexBuffer(mesh->m_indexBufferSize);

        std::vector<uint8_t> vertices(mesh->m_vertexBufferSize);
        uint8_t* pVertex = vertices.data();
        for (uint32_t vertexIndex = 0; vertexIndex < meshData->mNumVertices; vertexIndex++)
        {
            if (meshData->HasPositions())
            {
                *reinterpret_cast<glm::vec3*>(pVertex) = ToVec3(meshData->mVertices[vertexIndex]);
                pVertex += sizeof(glm::vec3);
            }
            if (meshData->HasNormals())
            {
                glm::vec3 normal = ToVec3(meshData->mNormals[vertexIndex]);
                *reinterpret_cast<glm::vec3*>(pVertex) = normal;
                pVertex += sizeof(glm::vec3);

                if (meshData->HasTangentsAndBitangents())
                {
                    glm::vec3 tangent = ToVec3(meshData->mTangents[vertexIndex]);
                    glm::vec3 bitangent = ToVec3(meshData->mBitangents[vertexIndex]);
                    float handness = 1.0f;
                    if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f)
                    {
                        handness = -1.0f;
                    }
                    *reinterpret_cast<glm::vec4*>(pVertex) = glm::vec4(tangent, handness);
                    pVertex += sizeof(glm::vec4);
                }
            }
            if (uint32_t numUVChannels = meshData->GetNumUVChannels())
            {
                for (uint32_t channelIndex = 0; channelIndex < numUVChannels; channelIndex++)
                {
                    *reinterpret_cast<glm::vec2*>(pVertex) = glm::vec2(
                        meshData->mTextureCoords[channelIndex][vertexIndex][0],
                        meshData->mTextureCoords[channelIndex][vertexIndex][1]
                    );
                    pVertex += sizeof(glm::vec2);
                }
            }
            if (uint32_t numColorChannel = meshData->GetNumColorChannels())
            {
                *reinterpret_cast<glm::vec4*>(pVertex) = ToVec4(meshData->mColors[0][vertexIndex]);
                pVertex += sizeof(glm::vec4);
            }
        }

        mesh->m_indices.reserve(meshData->mNumFaces * 3);
        for (uint32_t faceIndex = 0; faceIndex < meshData->mNumFaces; faceIndex++)
        {
            assert(meshData->mFaces[faceIndex].mNumIndices == 3);
            mesh->m_indices.push_back(meshData->mFaces[faceIndex].mIndices[0]);
            mesh->m_indices.push_back(meshData->mFaces[faceIndex].mIndices[1]);
            mesh->m_indices.push_back(meshData->mFaces[faceIndex].mIndices[2]);
        }

        mesh->m_vertexBuffer->Write(vertices.data(), mesh->m_vertexBufferOffset, mesh->m_vertexBufferSize);
        mesh->m_indexBuffer->Write(mesh->m_indices.data(), mesh->m_indexBufferOffset, mesh->m_indexBufferSize);

        mesh->m_material = m_materials[meshData->mMaterialIndex];
        mesh->m_aabb.m_minCorner = ToVec3(meshData->mAABB.mMin);
        mesh->m_aabb.m_maxCorner = ToVec3(meshData->mAABB.mMax);
        return true;
    }

    Ref<VulkanTexture> CreateTexture2DFromFile(const aiMaterial* materialData, aiTextureType textureType, unsigned int index);
    bool InitMaterial(VulkanMaterial* material, const aiMaterial* materialData)
    {
        aiString name;
        materialData->Get(AI_MATKEY_NAME, name);
        // Metallic/Roughness Workflow
        // ---------------------------
        // Base RGBA color factor. Will be multiplied by final base color texture values if extant
        // Note: Importers may choose to copy this into AI_MATKEY_COLOR_DIFFUSE for compatibility
        // with renderers and formats that do not support Metallic/Roughness PBR
        material->m_baseColorTexture = CreateTexture2DFromFile(materialData, aiTextureType_DIFFUSE, 0);
        if (!material->m_baseColorTexture)
        {
            material->m_baseColorTexture = CreateTexture2DFromFile(materialData, AI_MATKEY_BASE_COLOR_TEXTURE);
        }
        return true;
    }

    void InitNodes(VulkanSceneNode* node, const aiNode* nodeData)
    {
        node->m_transform = ToMat4(nodeData->mTransformation);
        for (unsigned int i = 0; i < nodeData->mNumMeshes; i++)
        {
            node->m_meshes.push_back(m_meshes[nodeData->mMeshes[i]]);
        }
        for (unsigned int i = 0; i < nodeData->mNumChildren; i++)
        {
            Ref<VulkanSceneNode> child = MakeRefCounted<VulkanSceneNode>(node, nodeData->mName.C_Str());
            InitNodes(child.get(), nodeData->mChildren[i]);
            node->AddChild(child);
        }
    }
};

bool VulkanScene::Import(const Path& filePath)
{
    Ref<VulkanAsset> asset = MakeRefCounted<VulkanAsset>(this);
    if (asset->Import(filePath))
    {
        m_meshes.insert(m_meshes.end(),
            asset->m_meshes.begin(), asset->m_meshes.end()
        );
        m_materials.insert(m_materials.end(),
            asset->m_materials.begin(), asset->m_materials.end()
        );
        m_rootNode->AddChild(asset->m_rootNode);
        return true;
    }
    else
    {
        return false;
    }
}

BoundingBox VulkanScene::GetBoundingBox() const
{
    return m_rootNode->GetBoundingBox();
}

VulkanSceneNode::VulkanSceneNode(VulkanSceneNode* parent, std::string_view name) :
    m_parent(parent),
    m_name(name)
{
    m_transform = glm::identity<glm::mat4>();
}

VulkanSceneNode::~VulkanSceneNode()
{
}

void VulkanSceneNode::AddChild(Ref<VulkanSceneNode> childNode)
{
    m_children.push_back(childNode);
}

BoundingBox GetBoundingBoxRecursive(const VulkanSceneNode* node, glm::mat4 transform)
{
    BoundingBox nodeBox = {};
    transform = transform * node->m_transform;
    for (uint32_t i = 0; i < node->m_meshes.size(); ++i)
    {
        VulkanMesh* mesh = node->m_meshes[i].get();
        BoundingBox meshBox = {};
        for (int i = 0; i < 8; i++)
        {
            glm::vec3 worldCorner = transform * glm::vec4(mesh->m_aabb.Corner(i), 1.0f);
            meshBox.m_minCorner = glm::min(meshBox.m_minCorner, worldCorner);
            meshBox.m_maxCorner = glm::max(meshBox.m_maxCorner, worldCorner);
        }
        nodeBox = Union(nodeBox, meshBox);
    }

    for (auto& child : node->m_children)
    {
        BoundingBox childBoundingBox = GetBoundingBoxRecursive(child.get(), transform);
        nodeBox = Union(nodeBox, childBoundingBox);
    }

    return nodeBox;
}

BoundingBox VulkanSceneNode::GetBoundingBox() const
{
    glm::mat4 transform = glm::identity<glm::mat4>();
    VulkanSceneNode* parentNode = m_parent;
    while (parentNode != nullptr)
    {
        transform = parentNode->m_transform * transform;
        parentNode = parentNode->m_parent;
    }

    return GetBoundingBoxRecursive(this, transform);
}

VulkanMesh::VulkanMesh(VulkanScene* scene, std::string_view name) :
    m_scene(scene),
    m_name(name)
{
}

VulkanMesh::~VulkanMesh()
{
}

VulkanMaterial::VulkanMaterial(VulkanScene* scene, std::string_view name) :
    m_scene(scene),
    m_name(name)
{
}

VulkanMaterial::~VulkanMaterial()
{
}

VkSamplerAddressMode GetSamplerAddressMode(aiTextureMapMode mapMode)
{
    switch (mapMode)
    {
    case aiTextureMapMode_Wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case aiTextureMapMode_Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case aiTextureMapMode_Decal: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case aiTextureMapMode_Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

Ref<VulkanTexture> VulkanAsset::CreateTexture2DFromFile(const aiMaterial* materialData, aiTextureType textureType, unsigned int index)
{
    VulkanDevice* device = m_scene->m_device.get();
    aiString path;
    aiTextureMapping mapping;
    uint32_t uvIndex = 0;
    ai_real blend = 0.0f;
    aiTextureOp op;
    aiTextureMapMode mapMode[3]; // UVW

    bool bGenerateMipmaps = true;

    materialData->GetTexture(textureType,
        index,
        &path,
        &mapping,
        &uvIndex,
        &blend,
        &op,
        mapMode);

    Ref<VulkanTexture> texture = MakeRefCounted<VulkanTexture>();
    texture->filePath = m_baseDir / (const char8_t*)path.C_Str();
    if (auto iter = m_images.find(texture->filePath); iter != m_images.end())
    {
        texture->image = iter->second;
    }
    else
    {
        texture->image = VulkanImage::CreateImage2DFromFile(device, texture->filePath, bGenerateMipmaps);
        m_images[texture->filePath] = texture->image;
    }

    static_assert(TextureMappingUV == aiTextureMapping_UV);
    static_assert(TextureMappingSphere == aiTextureMapping_SPHERE);
    static_assert(TextureMappingCylinder == aiTextureMapping_CYLINDER);
    static_assert(TextureMappingBox == aiTextureMapping_BOX);
    static_assert(TextureMappingPlane == aiTextureMapping_PLANE);
    static_assert(TextureMappingOther == aiTextureMapping_OTHER);
    texture->mapping = static_cast<TextureMapping>(mapping);
    texture->texCoordIndex = uvIndex;
    texture->blend = blend;
    static_assert(TextureOpMultiply == aiTextureOp_Multiply);
    static_assert(TextureOpAdd == aiTextureOp_Add);
    static_assert(TextureOpSubtract == aiTextureOp_Subtract);
    static_assert(TextureOpDivide == aiTextureOp_Divide);
    static_assert(TextureOpSmoothAdd == aiTextureOp_SmoothAdd);
    static_assert(TextureOpSignedAdd == aiTextureOp_SignedAdd);
    texture->op = static_cast<TextureOp>(op);
    texture->addressModeU = GetSamplerAddressMode(mapMode[0]);
    texture->addressModeV = GetSamplerAddressMode(mapMode[1]);
    texture->addressModeW = GetSamplerAddressMode(mapMode[2]);

    return texture;
}
