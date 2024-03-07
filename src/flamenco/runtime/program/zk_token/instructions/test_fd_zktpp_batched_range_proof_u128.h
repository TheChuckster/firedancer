#ifndef HEADER_test_fd_zktpp_batched_range_proof_u128_h
#define HEADER_test_fd_zktpp_batched_range_proof_u128_h

#include "../fd_zktpp_private.h"

// v1.17.13: 01356526b2ee92f9f81468f076bb0d7e20912e8bf6282eef89535a49de00d486a2def6a455e8b93665ebec89be812f64ce2c8d001ace008052ca1ab81220d46f0d01000103be5b54cdb01762497c7fd98bfcaaec1d2a2cad1c2bb5134857b68f0214935ebb8b49da000e928676abdba5fad12cdc793702c3cae95c2eb468808ca94a4bb0730863ba8dd9c4c2fb174a05cba27e2a2cd623573d79e90b35b579fc0d0000000006080c3086c490007d88e243d799354a1ee234dfdb7f5f7d1c4a3e8b0263edac0102020100e90709d01237233159ce1d03b60584908f89191739fae2ccf2afab53a50f969f4eda12e27e15681cca6af9c573ec822c7a4ff1037ef5a929ebd86cdb060170020a1601c49560d2152ef4e657dd39190be3175b9d15a29ada9aa1d168d4fec53dc1a970d475b1bc2801c967a64a099c2a5cc154780940235c78ac889544635314ceb254000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040102010000000003ea7b3695740b168afa889c293f3173c160b3bcb2d895764f603642cf317790a18165de4651dcfcca59f5fb798568eec30f6363c8bc52690cc62d5d59d73ae0fd6d6f1799feac39f9f81daec8c17b9c6eaf628cf3b2bc5f5494a05962b8f3c6bdcc2f9113e1e6828dbd7a06bd5904f0e2e36c7efb5e3a2d3e043856476bc2423e7dcbebeca3f3e58ad8284a76c687e679ce6802dc3f7bd2420f15b517fd2b3008aa04ede6f30a4d092eab9bd31ca65019f1abda573e561d0a0d9e3ef0020400351abb1f943eb19d9462628591d826df0558d8b422d0cd10317ace85ca43a1d02ce24d3bca0d5466e039571abb102b6637dc64f276f704de3d723e851a98f6a4e7a668af3d1ec86f2831b4cf8bf56cefa842239613553414a5cb58c398633704256c803c3fb92a96769638a3adeb0755851f8af6b0c052e390ceecfe40ea36d070a7a547f7191ddc2dfa7388e0394e9799a03788996bc870bd564a9fd4978c43226be25ac7b56feded658ad8eded6e29b7daf88ae175eaec43a4683eb9b4ea71e5e6b04897ed3b8f328fa275e5db540e99f5a1198f78d4478191db52b307b8b42ba13cab64c338035fb61aa01be9d19cd5fa3c336b08750372cb9b020af67b535924a8f78f244d3bf695aafac8bc22e3f19edc9d20cb88a986dd997265c65b86a306b6f2e3565413ce040342a84542fdcde368cd35f1b6d22b8af05474e1492071cee6159c7a9cbf72d08e28cecee6d7d8a34a96e6636bce9aca0f03d6561cf16ce68efd3a16c92e8a79c0645087d1b1df14c65c0d71fb6aa4bbb666f167c6b48bca8ec4e34b5f70a4522609f4add869d997d3448a54693fa3174910698570227b2ae1e51f280816b59655a62c3cab69d034860771d15a9f9ab6391bfb5e03d3a928d8843df67de8b3c1aa96b26dc182facfe3cc580188f393035bc0dc47c1c0c7b99a799c954aa0213cf804c8ae563d50a07d95053fca38bab6ce49502b36b0ab96c923297a16e0b45cfb878c168cfa6dfcf22055c360427237d287109cdb90d
// v1.17.6:  014f5b72c446b5de04da8c8006cf5544efb762d64275a832ff4d11de73883cd52c1390ddbfd15858144ad3a54718e22ed07fd6033b8a4d34e5468b4c015a611b0801000103be5b54cdb01762497c7fd98bfcaaec1d2a2cad1c2bb5134857b68f0214935ebb59d455304d5dc642b79f5ae491b0666059bd0a10d1c2351657db2a59ef951a370863ba8dd9c4c2fb174a05cba27e2a2cd623573d79e90b35b579fc0d00000000197766de85ba05d13f147df3ceab522b0b0973402f38f9e5e4c0d3d25c4e92900102020100e9070952461434fb059f202ef7bd99c98b52c1cd891e2e5327635cd9559a7776cb451c3a95bc75987b0c81200aff5638ed1c92f9a47b710ebd3a37b50501ba782c1f68ca8e6d42729dfd4f1b5c4c58c8cb8ba61880f082526fe693de39b0c89ddc2c3f9a343ce1bb418548223e89d5a72e0eb8d62859bbe2bcb4285baf82b2ac2f7b520000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000401020100000000016b2921b17d785fd5bb5787b1cd9a0fe311a846127ef58510218e7b876ede659b2c1b37f35f06589d6cefb0c78038661ad3982e805ede7793d625354624f6915c093780d517472ef47e0132238d4097a953c8d26c4b7b67ce421712fd69900249a127d363ea7c3798c16385e900f6b8c1c8868f206b61bc9ce3bceb02d67f919c1cde783399f7a0a1991b8f5c5866457ec9fafbe4981db5154e70d625ebcea03a3da05fd404a4d280cc1b8346929c6dbd141dae03a78952e222b0613850d8d07c3e3644d6d94333efb7bf3e97c7f00d8c594c4bd5191334e80f1e556234faa096008ebd54a563cee5a977c7f21409a28a72cc18d534a2f56064d646fc87c9f0bfe853ea20f65dffc1eb65928c692449bb45b6aa5c8693c5b01c3dc7cccbf763d7607790bda089f0c40b114740292b8b453277fae7c9e216fb6d629bd99f37f723e8ffa3227f0e2c0d337f5bfdca2e473db76019e144f06fbfe8890438a1adf4636cb87b2feb884241a01955f58119bfe560b21d364ae193048118330aa480e0916a5c921d3306f23548af080dfdd8caeadda350c118628da6a8fe52d82ce0437a40293fba4c3fc823efff1d56e8195425d5f0a1687d9a64603fca35962b3442354b86d50010768e1fc7138fdefd7c670b5a44644821b12a07d85e77cb232db3a585db83d1d263128bdbfc9709960d7ddd7e330099afd6745ab4526357f1516360cd40409d721067061cb0f798d2291341f9782e0a9ab3d927709997a9d49156f92c89f9a28555075b6ad6cc09093ad7d2942ea7258f1349dc7463042a2b26e1e20216b641581a627f93f260032eb2229aba790b44799a3e3e1ed64d74e979f1314fd32dfa0e9f7eca5cefcaeb2eb45b3de02a3cf078ab1bcb7b10afe3fc65562320eecb397bff7c5eb3b3878042ff079dbb7c7981f0f32612713301bb6c4163bdb9f365e4afdfe32950d8e429b73a5f541f75ce0692bbef52ea3bebc96b43407aa537439f60f31c494e97d8a0be316d31623820a74a95498f28cefbf45e90e02
// note: v1.17.6 is incompatible, breaking change in v1.17.7
static char *
tx_batched_range_proof_u128[] = {
  "01",
  "356526b2ee92f9f81468f076bb0d7e20912e8bf6282eef89535a49de00d486a2def6a455e8b93665ebec89be812f64ce2c8d001ace008052ca1ab81220d46f0d",
  "01000103",
  "be5b54cdb01762497c7fd98bfcaaec1d2a2cad1c2bb5134857b68f0214935ebb",
  "8b49da000e928676abdba5fad12cdc793702c3cae95c2eb468808ca94a4bb073",
  "0863ba8dd9c4c2fb174a05cba27e2a2cd623573d79e90b35b579fc0d00000000",
  "06080c3086c490007d88e243d799354a1ee234dfdb7f5f7d1c4a3e8b0263edac",
  "01",
  "02020100e907",
  "09", // VerifyBatchedRangeProof128
  "d01237233159ce1d03b60584908f89191739fae2ccf2afab53a50f969f4eda12", // context
  "e27e15681cca6af9c573ec822c7a4ff1037ef5a929ebd86cdb060170020a1601",
  "c49560d2152ef4e657dd39190be3175b9d15a29ada9aa1d168d4fec53dc1a970",
  "d475b1bc2801c967a64a099c2a5cc154780940235c78ac889544635314ceb254",
  "0000000000000000000000000000000000000000000000000000000000000000",
  "0000000000000000000000000000000000000000000000000000000000000000",
  "0000000000000000000000000000000000000000000000000000000000000000",
  "0000000000000000000000000000000000000000000000000000000000000000",
  "4010201000000000",
  "3ea7b3695740b168afa889c293f3173c160b3bcb2d895764f603642cf317790a", // proof
  "18165de4651dcfcca59f5fb798568eec30f6363c8bc52690cc62d5d59d73ae0f",
  "d6d6f1799feac39f9f81daec8c17b9c6eaf628cf3b2bc5f5494a05962b8f3c6b",
  "dcc2f9113e1e6828dbd7a06bd5904f0e2e36c7efb5e3a2d3e043856476bc2423",
  "e7dcbebeca3f3e58ad8284a76c687e679ce6802dc3f7bd2420f15b517fd2b300",
  "8aa04ede6f30a4d092eab9bd31ca65019f1abda573e561d0a0d9e3ef00204003",
  "51abb1f943eb19d9462628591d826df0558d8b422d0cd10317ace85ca43a1d02",
  "ce24d3bca0d5466e039571abb102b6637dc64f276f704de3d723e851a98f6a4e",
  "7a668af3d1ec86f2831b4cf8bf56cefa842239613553414a5cb58c3986337042",
  "56c803c3fb92a96769638a3adeb0755851f8af6b0c052e390ceecfe40ea36d07",
  "0a7a547f7191ddc2dfa7388e0394e9799a03788996bc870bd564a9fd4978c432",
  "26be25ac7b56feded658ad8eded6e29b7daf88ae175eaec43a4683eb9b4ea71e",
  "5e6b04897ed3b8f328fa275e5db540e99f5a1198f78d4478191db52b307b8b42",
  "ba13cab64c338035fb61aa01be9d19cd5fa3c336b08750372cb9b020af67b535",
  "924a8f78f244d3bf695aafac8bc22e3f19edc9d20cb88a986dd997265c65b86a",
  "306b6f2e3565413ce040342a84542fdcde368cd35f1b6d22b8af05474e149207",
  "1cee6159c7a9cbf72d08e28cecee6d7d8a34a96e6636bce9aca0f03d6561cf16",
  "ce68efd3a16c92e8a79c0645087d1b1df14c65c0d71fb6aa4bbb666f167c6b48",
  "bca8ec4e34b5f70a4522609f4add869d997d3448a54693fa3174910698570227",
  "b2ae1e51f280816b59655a62c3cab69d034860771d15a9f9ab6391bfb5e03d3a",
  "928d8843df67de8b3c1aa96b26dc182facfe3cc580188f393035bc0dc47c1c0c",
  "7b99a799c954aa0213cf804c8ae563d50a07d95053fca38bab6ce49502b36b0a",
  "b96c923297a16e0b45cfb878c168cfa6dfcf22055c360427237d287109cdb90d",
};

const ulong instr_offset_batched_range_proof_u128 = 204;

#endif /* HEADER_test_fd_zktpp_batched_range_proof_u128_h */