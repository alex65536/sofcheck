#include "eval/feat/feat.h"

#include <gtest/gtest.h>
#include <json/json.h>

#include <sstream>

#include "util/strutil.h"

using SoFEval::Feat::Features;
using SoFEval::Feat::WeightVec;

constexpr const char *SRC_JSON = R"JSON(
[
    {"first": -41},
    {"second": [53, 143, 55, -39, 71]},
    {"our_psq": {
        "type": "psq",
        "cost": [-129, 78, -44, -142, 5, 77],
        "pawn": [
              58, -78,   18,   90,  110, -122, -145,  -66,
              48, -57,  -71,  -30,  136, -120, -139,   30,
            -145,  54,   38,  -91, -130,  -87,   57,   39,
              47, -10,  -16,   74,  -49,   53, -134,  -17,
              48,  48,  -20,   90,    1,   26,   20,  -58,
              39,  68,   58,  140,  -42,   85,   16, -109,
             -10,  77, -120, -127,    1,  -25,  -57,   65,
              42, -32,  139,  -28,  -61,  -45,  -94,  -27
        ],
        "king": [
             -3,  134,   38,   62,   77,  108, 148,  127,
            -55,   58, -120,   63,   16, -115, -24,  108,
            145, -109,  -16, -126,   81,   63, -39,  -17,
             30,   72, -142,   38,  114,  121, -19,  140,
            -75,  -16,  -67,  102, -145, -110, -71, -147,
            -97,    7,   26,   52, -112,   13, 131,    6,
             34,   13,   -4,  -71,  -65,   31,  77,  -30,
            -52,  -84,  -78, -144,  137,  -44, -75,  114
        ],
        "knight": [
              71,  116,  -29,   39,   17,  129,  -98,   52,
             -91,  -50,  135,  102,   -6,  -43,  123, -115,
             135,  -95,  104,  -63,  144,  144,  -49,  -66,
             -98, -139, -125,   26,  105,  117, -135,   -7,
             -58,  -17,   63, -107,  142,  107,   13,  143,
            -118, -136,  -70,   60, -109,   91,  -20,   38,
              37,   44,  -74,   25, -144, -120,  -75, -118,
             123,    2,  113,  100,  145,   -2,  113,   50
        ],
        "bishop": [
            -81,  -50,  -66,   59,   47,  114,  147, -142,
             56,  120,   44, -127,   81,   57, -131,  -75,
            143,  -43,    0,  -22, -102,   48,  144,   -5,
            -44,  -90, -124,   44,   92,   37,   -4, -113,
             -5,  -76,   75,  143,  119,   50, -134,   53,
            -12,  123,  -12,   -4,   65, -102,  -46,  -46,
            -72, -136,  -94,  -68,  135,  148,   78,  138,
            122,   32,   -6, -140, -117,  102, -138,   54
        ],
        "rook": [
            -111,  103, -37,  122,  26,  127,  -50,  53,
             -15,  -27, 138,   -3, -72,   93, -141,  61,
             -32,   73,   3,   38,  78,  -28,  137,  69,
            -138,   13,  40,  -59, -99, -126,   85,  41,
            -131,  -94,  35,  104, -67, -150,  -82, -89,
             116, -113,  -8,  144, 126,  -71,  109, -53,
            -144,   24, -22,  -97,  83,  -77,   57, -51,
              57,  102,  29, -101,  35, -150, -105, -82
        ],
        "queen": [
             70,   -9,  -66,  -40,   26, -146, -80, -71,
            127,  127, -127,  109,   99,  149,  28,  57,
            -28, -150,   55,   38, -142, -116,  18,  80,
            -71,   25,  -28,  -66, -110,  -40, -44,  47,
            -72,   87,  -71,  -77,   75,  -82, -44,   7,
             40,  -55,  -45,  102, -103,   39, 116, -90,
             11, -113,   54,   -8,  -13,  116,  24,  25,
              4, -149,  -35, -101,   40,   35, -91,  91
        ],
        "king_end": [
            -76,   74,  103,  139, -138,   86, -125,   72,
            -61,  120,   66,  -63,  -96,   40, -130, -125,
            -17,  130,   78, -134,   86,  107,  -92,   -9,
             19,    5,  -47,  -11,  123, -118,   84,  -94,
             42,   73,  -35,  -13,  -22, -125, -136,  -57,
            100,   12,   57, -109,  -83,  140,  138,  -52,
             52,  137, -119,   69,  -39,  -10,   -5,   58,
            138, -130,  -55,  -47,  -84,  -85,   15, -121
        ]
    }},
    {"a": 49},
    {"some_array": [56, -15, 126, -35]}
]
)JSON";

Json::Value featuresSaveToJson(const Features &features) {
  Json::Value result;
  features.save(result);
  return result;
}

std::string featuresSaveToStr(const Features &features) {
  std::ostringstream stream;
  features.print(stream);
  return stream.str();
}

TEST(SoFEval_Feat, Feat_LoadSave) {
  std::istringstream featuresIn(SRC_JSON);
  std::istringstream featuresJsonIn(SRC_JSON);
  Json::Value featuresJson;
  featuresJsonIn >> featuresJson;

  auto features = Features::load(featuresIn).unwrap();

  {
    // Check that load from `std::istream` and load from `Json::Value` yield the same result
    auto featuresFromJson = Features::load(featuresJson).unwrap();
    EXPECT_EQ(featuresSaveToJson(features), featuresSaveToJson(featuresFromJson));
  }

  // Check that `save()` yields the same JSON as loaded
  EXPECT_EQ(featuresSaveToJson(features), featuresJson);

  // Check that the formatted output is identical to the input
  EXPECT_EQ(SoFUtil::trimmed(SRC_JSON) + "\n", featuresSaveToStr(features));
}

TEST(SoFEval_Feat, Feat_Weights) {
  std::istringstream featuresIn(SRC_JSON);
  auto features = Features::load(featuresIn).unwrap();

  EXPECT_EQ(features.count(), 465);
  WeightVec weights = features.extract();
  ASSERT_EQ(weights.size(), 465);
  EXPECT_EQ(weights[0], -41);
  EXPECT_EQ(weights[3], 55);
  EXPECT_EQ(weights[9], -142);
  EXPECT_EQ(weights[22], -71);
  EXPECT_EQ(weights[463], 126);

  weights[0] = -1;
  weights[3] = 20;
  weights[9] = -71;
  weights[22] = -10000;
  weights[463] = 88;
  features.apply(weights);
  EXPECT_EQ(features.extract(), weights);
}

TEST(SoFEval_Feat, Feat_Names) {
  std::istringstream featuresIn(SRC_JSON);
  auto features = Features::load(featuresIn).unwrap();

  EXPECT_EQ(features.count(), 465);
  const auto names = features.names();
  ASSERT_EQ(names.size(), 465);
  EXPECT_EQ(names[0].name, "first");
  EXPECT_EQ(names[3].name, "second.2");
  EXPECT_EQ(names[10].name, "our_psq.cost.4");
  EXPECT_EQ(names[167].name, "our_psq.knight.27");
  EXPECT_EQ(names[203].name, "our_psq.knight.63");
  EXPECT_EQ(names[204].name, "our_psq.bishop.0");
  EXPECT_EQ(names[463].name, "some_array.2");
}

TEST(SoFEval_Feat, Feat_Bundles) {
  std::istringstream featuresIn(SRC_JSON);
  auto features = Features::load(featuresIn).unwrap();

  ASSERT_EQ(features.bundles().size(), 5);

  EXPECT_EQ(features.bundles()[0].name().name, "first");
  EXPECT_EQ(features.bundles()[0].name().offset, 0);
  EXPECT_NE(features.bundles()[0].asSingle(), nullptr);

  EXPECT_EQ(features.bundles()[1].name().name, "second");
  EXPECT_EQ(features.bundles()[1].name().offset, 1);
  EXPECT_NE(features.bundles()[1].asArray(), nullptr);
  EXPECT_EQ(features.bundles()[1].asSingle(), nullptr);

  EXPECT_EQ(features.bundles()[2].name().name, "our_psq");
  EXPECT_EQ(features.bundles()[2].name().offset, 6);
  EXPECT_NE(features.bundles()[2].asPsq(), nullptr);

  EXPECT_EQ(features.bundles()[3].name().name, "a");
  EXPECT_EQ(features.bundles()[3].name().offset, 460);
  EXPECT_NE(features.bundles()[3].asSingle(), nullptr);

  EXPECT_EQ(features.bundles()[4].name().name, "some_array");
  EXPECT_EQ(features.bundles()[4].name().offset, 461);
  EXPECT_NE(features.bundles()[4].asArray(), nullptr);
}
