#include "pch.h"

#include "GpuMemoryBuffer.h"
#include "GraphicsDevice/IGraphicsDevice.h"
#include "GraphicsDevice/ITexture2D.h"
#include "GraphicsDeviceContainer.h"
#include "Size.h"
#include "UnityVideoTrackSource.h"
#include "VideoFrameAdapter.h"
#include "VideoFrameUtil.h"

namespace unity
{
namespace webrtc
{

    class GpuMemoryBufferTest : public testing::TestWithParam<UnityGfxRenderer>
    {
    public:
        explicit GpuMemoryBufferTest()
            : container_(CreateGraphicsDeviceContainer(GetParam()))
            , device_(container_->device())
        {
        }

    protected:
        void SetUp() override
        {
            if (!device_)
                GTEST_SKIP() << "The graphics driver is not installed on the device.";

            std::unique_ptr<ITexture2D> texture(device_->CreateDefaultTextureV(kWidth, kHeight, kFormat));
            if (!texture)
                GTEST_SKIP() << "The graphics driver cannot create a texture resource.";
        }
        std::unique_ptr<GraphicsDeviceContainer> container_;
        IGraphicsDevice* device_;
        const uint32_t kWidth = 256;
        const uint32_t kHeight = 256;
        const UnityRenderingExtTextureFormat kFormat = kUnityRenderingExtFormatR8G8B8A8_SRGB;
    };

    TEST_P(GpuMemoryBufferTest, WidthAndHeight)
    {
        const Size kSize(kWidth, kHeight);
        IGraphicsDevice* device = container_->device();
        std::unique_ptr<const ITexture2D> texture(
            device->CreateDefaultTextureV(kSize.width(), kSize.height(), kFormat));
        auto testFrame = CreateTestFrame(device, texture.get(), kFormat);

        auto frame = VideoFrameAdapter::CreateVideoFrame(testFrame);
        auto buffer = frame.video_frame_buffer();

        EXPECT_EQ(buffer->width(), kSize.width());
        EXPECT_EQ(buffer->height(), kSize.height());
    }

    TEST_P(GpuMemoryBufferTest, Scale)
    {
        const Size kSize(kWidth * 2, kHeight * 2);
        const Size kSize2(kWidth, kHeight);
        std::unique_ptr<const ITexture2D> texture(
            device_->CreateDefaultTextureV(kSize.width(), kSize.height(), kFormat));
        auto testFrame = CreateTestFrame(device_, texture.get(), kFormat);
        auto frame = VideoFrameAdapter::CreateVideoFrame(testFrame);
        auto buffer = frame.video_frame_buffer();
        auto buffer2 = buffer->Scale(kSize2.width(), kSize2.height());

        EXPECT_NE(buffer2, nullptr);
        EXPECT_EQ(buffer2->type(), buffer->type());
        EXPECT_EQ(buffer2->width(), kSize2.width());
        EXPECT_EQ(buffer2->height(), kSize2.height());

        auto i420Buffer = buffer2->ToI420();

        EXPECT_NE(i420Buffer, nullptr);
        EXPECT_EQ(i420Buffer->type(), VideoFrameBuffer::Type::kI420);
        EXPECT_EQ(i420Buffer->width(), kSize2.width());
        EXPECT_EQ(i420Buffer->height(), kSize2.height());
    }

    INSTANTIATE_TEST_SUITE_P(GfxDevice, GpuMemoryBufferTest, testing::ValuesIn(supportedGfxDevices));

} // end namespace webrtc
} // end namespace unity