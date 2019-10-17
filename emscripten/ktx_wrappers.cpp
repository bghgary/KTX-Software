#include <emscripten/bind.h>
#include <ktx.h>

#include <iostream>

using namespace emscripten;

namespace ktx_wrappers
{
    class texture
    {
    public:
        texture(texture&) = delete;
        texture(texture&& other) = default;

        static const uint32_t KTX_TF_ETC1 = ::KTX_TF_ETC1;
        static const uint32_t KTX_TF_BC1 = ::KTX_TF_BC1;
        static const uint32_t KTX_TF_BC4 = ::KTX_TF_BC4;
        static const uint32_t KTX_TF_PVRTC1_4_OPAQUE_ONLY = ::KTX_TF_PVRTC1_4_OPAQUE_ONLY;
        static const uint32_t KTX_TF_BC7_M6_OPAQUE_ONLY = ::KTX_TF_BC7_M6_OPAQUE_ONLY;
        static const uint32_t KTX_TF_ETC2 = ::KTX_TF_ETC2;
        static const uint32_t KTX_TF_BC3 = ::KTX_TF_BC3;
        static const uint32_t KTX_TF_BC5 = ::KTX_TF_BC5;

        static texture createFromMemory(const emscripten::val& data)
        {
            std::vector<uint8_t> bytes{};
            bytes.resize(data["byteLength"].as<size_t>());
            emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
            emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
            memoryView.call<void>("set", data);

            ktxTexture* ptr = nullptr;
            KTX_error_code result = ktxTexture_CreateFromMemory(bytes.data(), bytes.size(), KTX_TEXTURE_CREATE_NO_FLAGS, &ptr);
            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to create from memory: " << result << std::endl;
                return texture(nullptr, {});
            }

            return texture(ptr, std::move(bytes));
        }

        // emscripten::val getData() const
        // {
        //     return emscripten::val(emscripten::typed_memory_view(ktxTexture_GetSize(m_ptr), ktxTexture_GetData(m_ptr)));
        // }

        uint32_t baseWidth() const
        {
            return m_ptr->baseWidth;
        }

        uint32_t baseHeight() const
        {
            return m_ptr->baseHeight;
        }

        void transcodeBasis(uint32_t fmt, uint32_t decodeFlags)
        {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: transcodeBasis is only supported for KTX2" << std::endl;
                return;
            }

            KTX_error_code result = ktxTexture2_TranscodeBasis(
                reinterpret_cast<ktxTexture2*>(m_ptr.get()),
                static_cast<ktx_texture_transcode_fmt_e>(fmt),
                static_cast<ktx_uint32_t>(decodeFlags));

            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to transcode: " << result << std::endl;
                return;
            }
        }

        emscripten::val glUpload()
        {
            GLuint texture = 0;
            GLenum target = 0;
            GLenum error = 0;
            KTX_error_code result = ktxTexture_GLUpload(m_ptr.get(), &texture, &target, &error);
            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to GL upload: " << result << std::endl;
            }

            emscripten::val ret = emscripten::val::object();
            ret.set("texture", texture);
            ret.set("target", target);
            ret.set("error", error);
            return std::move(ret);
        }

    private:
        texture(ktxTexture* ptr, std::vector<ktx_uint8_t> bytes)
            : m_ptr{ ptr, &destroy }
            , m_bytes{ std::move(bytes) }
        {
        }

        static void destroy(ktxTexture* ptr)
        {
            ktxTexture_Destroy(ptr);
        }

        std::unique_ptr<ktxTexture, decltype(&destroy)> m_ptr;
        std::vector<uint8_t> m_bytes;
    };
}

EMSCRIPTEN_BINDINGS(ktx_wrappers)
{
    class_<ktx_wrappers::texture>("ktxTexture")
        .class_property("KTX_TF_ETC1", &ktx_wrappers::texture::KTX_TF_ETC1)
        .class_property("KTX_TF_BC1", &ktx_wrappers::texture::KTX_TF_BC1)
        .class_property("KTX_TF_BC4", &ktx_wrappers::texture::KTX_TF_BC4)
        .class_property("KTX_TF_PVRTC1_4_OPAQUE_ONLY", &ktx_wrappers::texture::KTX_TF_PVRTC1_4_OPAQUE_ONLY)
        .class_property("KTX_TF_BC7_M6_OPAQUE_ONLY", &ktx_wrappers::texture::KTX_TF_BC7_M6_OPAQUE_ONLY)
        .class_property("KTX_TF_ETC2", &ktx_wrappers::texture::KTX_TF_ETC2)
        .class_property("KTX_TF_BC3", &ktx_wrappers::texture::KTX_TF_BC3)
        .class_property("KTX_TF_BC5", &ktx_wrappers::texture::KTX_TF_BC5)
        .class_function("createFromMemory", &ktx_wrappers::texture::createFromMemory)
        // .property("data", &ktx_wrappers::texture::getData)
        .property("baseWidth", &ktx_wrappers::texture::baseWidth)
        .property("baseHeight", &ktx_wrappers::texture::baseHeight)
        .function("transcodeBasis", &ktx_wrappers::texture::transcodeBasis)
        .function("glUpload", &ktx_wrappers::texture::glUpload)
    ;
}