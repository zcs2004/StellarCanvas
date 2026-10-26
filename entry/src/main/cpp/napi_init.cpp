#include <cstdint>
#include <string>

#include <hilog/log.h>
#include <napi/native_api.h>

namespace {

constexpr std::uint32_t kLogDomain = 0xD003F00;
constexpr const char *kLogTag = "StellarCanvas";

napi_value MakeString(napi_env env, const std::string &value)
{
    napi_value result = nullptr;
    napi_create_string_utf8(env, value.c_str(), value.size(), &result);
    return result;
}

napi_value GetNativeBuildInfo(napi_env env, napi_callback_info /* info */)
{
    std::string result = "C++17 / arm64-v8a / HarmonyOS";
#ifdef __OHOS__
    result += " / __OHOS__";
#endif
    return MakeString(env, result);
}

napi_value RunNativeSelfTest(napi_env env, napi_callback_info /* info */)
{
    constexpr std::uint32_t expected = 0x4B524954; // "KRIT"
    constexpr std::uint32_t actual =
        (static_cast<std::uint32_t>('K') << 24U) |
        (static_cast<std::uint32_t>('R') << 16U) |
        (static_cast<std::uint32_t>('I') << 8U) |
        static_cast<std::uint32_t>('T');
    const bool passed = actual == expected;

    OH_LOG_Print(LOG_APP, passed ? LOG_INFO : LOG_ERROR, kLogDomain, kLogTag,
                 "Native bridge self-test: %{public}s", passed ? "passed" : "failed");

    napi_value result = nullptr;
    napi_get_boolean(env, passed, &result);
    return result;
}

napi_value Init(napi_env env, napi_value exports)
{
    const napi_property_descriptor descriptors[] = {
        {"getNativeBuildInfo", nullptr, GetNativeBuildInfo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"runNativeSelfTest", nullptr, RunNativeSelfTest, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors);
    return exports;
}

} // namespace

static napi_module stellarCanvasModule = {
    1,
    0,
    nullptr,
    Init,
    "libstellarcanvas",
    nullptr,
    {0},
};

extern "C" __attribute__((constructor)) void RegisterStellarCanvasModule()
{
    napi_module_register(&stellarCanvasModule);
}
