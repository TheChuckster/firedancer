#ifndef HEADER_test_fd_zktpp_withdraw_h
#define HEADER_test_fd_zktpp_withdraw_h

#include "../fd_zktpp_private.h"

// v1.17.13: 019b4823279c0d37e6497791defd7046d40a8e935c9b7cbd27ca1855bbb0e0103412a0a79c925b7c4de7742c1cdd784b5777843d1ce25ef654886f1f5c7e158b0b01000103be5b54cdb01762497c7fd98bfcaaec1d2a2cad1c2bb5134857b68f0214935ebb86ddc6585609faef7c746983e0b198e432e9d1771d615ce2406e320c779f4a3d0863ba8dd9c4c2fb174a05cba27e2a2cd623573d79e90b35b579fc0d00000000585f0a1fbf89f180f658f1fb1e74a07c23a67f7d7a5cdd6f9893ab23392124ec0102020100e10702a0d0383c8e276e1748ac744b7106b4ea8ae1de3fc3879e59a01124209f4a6c0578af4e008119d21088f8caf95a34ec4de1b576ae5c83218a0bbf0871cff410625c11a15301bbea37af5afa4a9c1a46f4cfa3cdcfef54d545e5782bd77b077d480c9cebe7d0174a1e85ef0aabb60baf24349f4a3fb5861a60af6a0db78aee7759e4137adb4abdc2a7bc9676cf440f9a290328095a014e6fd07e1e4b27542c797d70ead0d25677fb63afef6ddc8cc8e956d0269907781e43607bc5d184ed2b2d5d065011f40018eb772eaa733a74870c5351f09fa7263ec98dfcec103133a4ec180b9b71743305a68386288995e8b5729e0e0f740b980112c8c1fc3b8574356e01ad2bea776d4db981b25f83a690e42bd788f4d967501496257e2600737a6d3409013461b33e6d507d43077d26fb62572cba5c0f94f23180b64b177c1bf2693a07a6cc749f8b527f2eca6631f450e5cfec867b95e23f90b272e35bb7a81a292421361399b98a026ca9bb04968768ca5ea7d45b3fa650968bb201f4d69f9b48c12e42fe3f41d4948e56ef8a5e82bfd5eacf05685ceed5d575cabc4f433e7ea8b3793e03e63adbb4ac2704fb84f1c2f559fe948aedbff39f2766dd17c3cd449f1a3cc17a4053dfcf2db254e4ba1394fa98cf72c1d072888efb90b27915b0046b8e09b1fc99a5f1c4c56f3a71892d619a1a86dc86af5c83797b79f6908fb9f3c6c702180146d38ac48298bf1f249917c71f8d906b9e325ff76271561e988f5b04d30d1ac044000e419cc0aec1559a14600d2c33010f30116a3adfbdb6a2e6a4c47032e82fe0139b7828af0e396c3ce0a4a389102dadc124a5496ea1c038aca9f07e716ecf46dcead6c2e4eb1993031fa7b7ed1fb9a9e9d33c8c022bf8762b86bbd33032518a1f3f26db4757fa017d1a054b4ad27b390f9c644edb7ea71aac81f4a2034464a7795909dcc071d3f08cb21a543431d6e116e2d7ee3d0128992d455ed177dae03174957f00cb8cdbf7e5d1325cd39cb65ee5b1bc545ad4e70b9b4ca44123da8318f45488e983ddad12757bbea27c3ef3582ab845d03c4486761541723a11fceef565bdc5d4686d623416b027f5cce7dd672660ee061ea5fe9d16932e627b4883ccd361189a0dec70ad35c7836da6c19446d8fce088a4033161467175be539c6b7682f87e3e3d64cbe834c56e810eeaeabe9cc2519664f2a98f10a179b34f3e4714b0cf2d1df5732b33a06e67f69d80f99252c96cc06395112bde38b827214ceb70d498a32c4c891f47d158ad177bb8bc9e52d43d065f6857f5ed21cd3e64a762ec696935bcf31a09915a6ea0802e1ac231d019e945badebb52d64b1c39094f9db14124b512a3d3dcf7abfc07dbca77b1b50723708ce5895bb123cef6220e
// v1.17.6:  011d0ed4e6957624f900d14ac6268df1906510f2ee151f47fc3b82661e5905d5c74256fc8f21dcca2124022934407c4a1fe3d21b693b782365d79da90da9086a0601000103be5b54cdb01762497c7fd98bfcaaec1d2a2cad1c2bb5134857b68f0214935ebb62e658c293c604459865e83503e2d1987f4dc07210e3a777477ee13843fcd2650863ba8dd9c4c2fb174a05cba27e2a2cd623573d79e90b35b579fc0d00000000abf366d889b959601fac56131767af36d26ee6edd04bb56d379662cd987db0400102020100e10702a0d0383c8e276e1748ac744b7106b4ea8ae1de3fc3879e59a01124209f4a6c05fc48053fcbaa1d227fb738ddd9dc6bf5e7329cab5c3c1277a5d877fa95995d042647444d6b3edeb2d4cfeedef404f203f8ad1f2d5a64530ad890bffab4861a66148e4ad8365c3c62d7574c69d53b5258bdbeb050c237c00e4209eac71d9c253eacf73a2a3a4e9502227acf32ba10f9f1c135389566ea2ca178e744c3bc75a318b0954a829a1e656d3c7ca1f38515c986a76bb4a49b09944c5ba430819a646924fc3dda84ec51635ab92a6df9a2ec136c5a529c7453b28dada24cd18a8dfaf21bfe445e02845835725abb176bfd333c14c6cb3888f18230f2411a9c661b5d0e04f74fa26ef82841bc0b4dbad76f8ab33becc8f3c6ad469649905fd105566b60022540bfd858a92c4254a43ed0f63a3677d6fc13ad17f5d98f1eb276a9d953660bb0d56710ff6c976e49e11ff6d71863469cc5e8982d060a203251b1e3e91fcb5a2a33b8ea9a708a9ae743bf1a81e3f218c82f06daa5da3a877dd0e86df80ce26d3e6b9ceedd999e2178907761c0ef76174756e99125176a5c9389ce9e642f631d9ec7fe1c6501160c77b13049ef181cc3968928bb344d2cf99c4b422b56e3ff2fd102f7771beb826325b37e11378138d11c83c937a4d859e7adad137d8cb5ce02fcf0a7853891fc4e864fe300fe2596d69e1d0395640154e9d8e5714d33ff580b703fbe2c61bfafc9b2a156b0f509b0ec5afc624a6fd3311800274bb246b6480ce671cb1fb977a3a06e2d56084d44d146def4b58da704ed1b0e4b662f00ad0d060a6f57484d749a9f1f4bd5638d55ce72f839622fcd83edcf48de0dfe6e552c787eb2c1174b57f9fd97fa93e97f04309f24e1fdb52f1a646ca152fdef3be9ad4a2ee57ecade0ed2c5b3eda0a38fee9f7bccdc16e2126555a1f58ea0c93c57c61bca3116ed2348fbf6da1ac51b6e2b048af7994505629876b045fead3844a8d6602c800ceec4abc96ab416ca49d382797670c4c2965fc9c94c5ee082854332100ff80733e0a6992dd66c1ab3f1a56e0fd985c323ee2422cb499a0caf97f1ab4f781a6024574b9a01e825494014ba3d8f008464bdb0337e14a0ea0f50617bbd3a0a9c21e77cbd1c415f790337e1c10f8b8f2ae76b2277815abfdae8debb6b9a470bf88b1db8054797001da0c4d1510c7424d2f3bd9d2a1f32fdfe6f2dab142f1c7f9ad03e405a4e34ca1fab11d00f3df1161885558ba805971425fabd539632cc5f0c5924afefce23d31de894e2c48fcefb1c4206a6adf1226acf7cb37fe39c3801f8af98316112c9c3b11c450b95f9b73d60b3d734eb4f78f2b3b9403e47c36e0c2ded07f7627a9a349d73a411c7448ab96829eef65e60dade5ab4eda76379c706
// note: v1.17.6 is incompatible, breaking change in v1.17.7
static char *
tx_withdraw[] = {
  "01",
  "9b4823279c0d37e6497791defd7046d40a8e935c9b7cbd27ca1855bbb0e0103412a0a79c925b7c4de7742c1cdd784b5777843d1ce25ef654886f1f5c7e158b0b",
  "01000103",
  "be5b54cdb01762497c7fd98bfcaaec1d2a2cad1c2bb5134857b68f0214935ebb",
  "86ddc6585609faef7c746983e0b198e432e9d1771d615ce2406e320c779f4a3d",
  "0863ba8dd9c4c2fb174a05cba27e2a2cd623573d79e90b35b579fc0d00000000",
  "585f0a1fbf89f180f658f1fb1e74a07c23a67f7d7a5cdd6f9893ab23392124ec",
  "01",
  "02020100e107",
  "02", // VerifyWithdraw
  "a0d0383c8e276e1748ac744b7106b4ea8ae1de3fc3879e59a01124209f4a6c05", // context
  "78af4e008119d21088f8caf95a34ec4de1b576ae5c83218a0bbf0871cff41062",
  "5c11a15301bbea37af5afa4a9c1a46f4cfa3cdcfef54d545e5782bd77b077d48",
  "0c9cebe7d0174a1e85ef0aabb60baf24349f4a3fb5861a60af6a0db78aee7759", // proof: - commitment
  "e4137adb4abdc2a7bc9676cf440f9a290328095a014e6fd07e1e4b27542c797d", // - equality_proof
  "70ead0d25677fb63afef6ddc8cc8e956d0269907781e43607bc5d184ed2b2d5d",
  "065011f40018eb772eaa733a74870c5351f09fa7263ec98dfcec103133a4ec18",
  "0b9b71743305a68386288995e8b5729e0e0f740b980112c8c1fc3b8574356e01",
  "ad2bea776d4db981b25f83a690e42bd788f4d967501496257e2600737a6d3409",
  "013461b33e6d507d43077d26fb62572cba5c0f94f23180b64b177c1bf2693a07",
  "a6cc749f8b527f2eca6631f450e5cfec867b95e23f90b272e35bb7a81a292421", // - range_proof_u64
  "361399b98a026ca9bb04968768ca5ea7d45b3fa650968bb201f4d69f9b48c12e",
  "42fe3f41d4948e56ef8a5e82bfd5eacf05685ceed5d575cabc4f433e7ea8b379",
  "3e03e63adbb4ac2704fb84f1c2f559fe948aedbff39f2766dd17c3cd449f1a3c",
  "c17a4053dfcf2db254e4ba1394fa98cf72c1d072888efb90b27915b0046b8e09",
  "b1fc99a5f1c4c56f3a71892d619a1a86dc86af5c83797b79f6908fb9f3c6c702",
  "180146d38ac48298bf1f249917c71f8d906b9e325ff76271561e988f5b04d30d",
  "1ac044000e419cc0aec1559a14600d2c33010f30116a3adfbdb6a2e6a4c47032",
  "e82fe0139b7828af0e396c3ce0a4a389102dadc124a5496ea1c038aca9f07e71",
  "6ecf46dcead6c2e4eb1993031fa7b7ed1fb9a9e9d33c8c022bf8762b86bbd330",
  "32518a1f3f26db4757fa017d1a054b4ad27b390f9c644edb7ea71aac81f4a203",
  "4464a7795909dcc071d3f08cb21a543431d6e116e2d7ee3d0128992d455ed177",
  "dae03174957f00cb8cdbf7e5d1325cd39cb65ee5b1bc545ad4e70b9b4ca44123",
  "da8318f45488e983ddad12757bbea27c3ef3582ab845d03c4486761541723a11",
  "fceef565bdc5d4686d623416b027f5cce7dd672660ee061ea5fe9d16932e627b",
  "4883ccd361189a0dec70ad35c7836da6c19446d8fce088a4033161467175be53",
  "9c6b7682f87e3e3d64cbe834c56e810eeaeabe9cc2519664f2a98f10a179b34f",
  "3e4714b0cf2d1df5732b33a06e67f69d80f99252c96cc06395112bde38b82721",
  "4ceb70d498a32c4c891f47d158ad177bb8bc9e52d43d065f6857f5ed21cd3e64",
  "a762ec696935bcf31a09915a6ea0802e1ac231d019e945badebb52d64b1c3909",
  "4f9db14124b512a3d3dcf7abfc07dbca77b1b50723708ce5895bb123cef6220e",
};

const ulong instr_offset_withdraw = 204;

#endif /* HEADER_test_fd_zktpp_withdraw_h */