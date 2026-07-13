# Override FindMetalTools to skip Metal support on macOS
# We use Vulkan via MoltenVK, so Metal graphics API in OpenXR is not needed

# Just mark as not found without searching
set(MetalTools_FOUND FALSE)
