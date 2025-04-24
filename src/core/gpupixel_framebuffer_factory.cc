/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#include "core/gpupixel_framebuffer_factory.h"
#include "utils/util.h"

namespace gpupixel {

FramebufferFactory::FramebufferFactory() {}

FramebufferFactory::~FramebufferFactory() {
  Clean();
}

std::shared_ptr<GPUPixelFramebuffer> FramebufferFactory::CreateFramebuffer(
    int width,
    int height,
    bool only_texture /* = false*/,
    const TextureAttributes texture_attributes /* = defaultTextureAttribure*/) {
  std::shared_ptr<GPUPixelFramebuffer> framebuffer_from_cache;
  std::string lookup_hash = GenerateUuid(width, height, only_texture, texture_attributes);
  int number_of_matching_framebuffers = 0;
  auto itc = framebuffer_type_counts_.find(lookup_hash);
  if (itc != framebuffer_type_counts_.end()) {
    number_of_matching_framebuffers = itc->second;
  }
  
  if (number_of_matching_framebuffers > 0) {
    int cur_framebuffer_id = number_of_matching_framebuffers - 1;
    while (!framebuffer_from_cache && cur_framebuffer_id >= 0) {
      auto it = framebuffers_.find(Util::StringFormat("%s-%ld", lookup_hash.c_str(), cur_framebuffer_id));
      if (it != framebuffers_.end()) {
        framebuffer_from_cache = it->second;
        framebuffers_.erase(it);
      } else {
        framebuffer_from_cache = 0;
      }
      cur_framebuffer_id--;
    }
    cur_framebuffer_id++;
    framebuffer_type_counts_[lookup_hash] = cur_framebuffer_id;
  }

  if (!framebuffer_from_cache) {
    framebuffer_from_cache = std::make_shared<GPUPixelFramebuffer>(
            width, height, only_texture, texture_attributes);
  }

  return framebuffer_from_cache;
}

std::string FramebufferFactory::GenerateUuid(
    int width,
    int height,
    bool only_texture,
    const TextureAttributes texture_attributes) const {
  if (only_texture) {
    return Util::StringFormat(
        "%.1dx%.1d-%d:%d:%d:%d:%d:%d:%d-NOFB", width, height,
        texture_attributes.minFilter, texture_attributes.magFilter,
        texture_attributes.wrapS, texture_attributes.wrapT,
        texture_attributes.internalFormat, texture_attributes.format,
        texture_attributes.type);
  } else {
    return Util::StringFormat(
        "%.1dx%.1d-%d:%d:%d:%d:%d:%d:%d", width, height,
        texture_attributes.minFilter, texture_attributes.magFilter,
        texture_attributes.wrapS, texture_attributes.wrapT,
        texture_attributes.internalFormat, texture_attributes.format,
        texture_attributes.type);
  }
}

std::shared_ptr<GPUPixelFramebuffer> FramebufferFactory::GetFramebufferByUuid(
    const std::string& hash) {
  return framebuffers_[hash];
}

void FramebufferFactory::Clean() {
  framebuffers_.clear();
  framebuffer_type_counts_.clear();
}

}  // namespace gpupixel
