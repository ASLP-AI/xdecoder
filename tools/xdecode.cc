// Copyright (c) 2016 Personal (Binbin Zhang)
// Created on 2018-06-12
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>

#include <limits>

#include "fst.h"
#include "wav.h"
#include "faster-decoder.h"
#include "parse-option.h"

int main(int argc, char* argv[]) {
  using xdecoder::ParseOptions;
  using xdecoder::FasterDecoder;
  using xdecoder::FasterDecoderOptions;
  using xdecoder::FeaturePipeline;
  using xdecoder::FeaturePipelineConfig;
  using xdecoder::Fst;
  using xdecoder::Tree;
  using xdecoder::Net;
  using xdecoder::Vector;
  using xdecoder::SymbolTable;
  using xdecoder::OnlineDecodable;
  using xdecoder::DecodableOptions;
  using xdecoder::WavReader;

  const char *usage = "\n";
  ParseOptions option(usage);

  FasterDecoderOptions decoder_options;
  DecodableOptions decodable_options;
  FeaturePipelineConfig feature_options;
  option.Register("beam", &decoder_options.beam,
                  "Decoding beam.  Larger->slower, more accurate.");
  option.Register("max-active", &decoder_options.max_active,
                 "Decoder max active states.  Larger->slower; more accurate");
  option.Register("acoustic-scale", &decodable_options.acoustic_scale,
                  "Acoustic scale for decoding");
  option.Register("skip", &decodable_options.skip,
                  "skip for decoding");
  option.Register("max-batch-size", &decodable_options.max_batch_size,
                  "max batch size for decoding");
  option.Register("num-bins", &feature_options.num_bins,
                  "Fbank dimension");
  option.Register("left-context", &feature_options.left_context,
                  "feature left context");
  option.Register("right-context", &feature_options.right_context,
                  "feature right context");
  option.Register("cmvn-file", &feature_options.cmvn_file,
                  "feature global cmvn file");
  option.Read(argc, argv);


  if (option.NumArgs() != 7) {
    option.PrintUsage();
    exit(1);
  }

  std::string hclg_file = option.GetArg(1);
  std::string tree_file = option.GetArg(2);
  std::string net_file = option.GetArg(3);
  std::string pdf_prior_file = option.GetArg(4);
  std::string word_file = option.GetArg(5);
  std::string wav_scp_file = option.GetArg(6);
  std::string result_file = option.GetArg(7);

  Fst fst(hclg_file);
  Tree tree(tree_file);
  Net net(net_file);
  Vector<float> pdf_prior;
  {
    std::ifstream is(pdf_prior_file, std::ifstream::binary);
    if (is.fail()) {
      ERROR("read file %s error, check!!!", pdf_prior_file.c_str());
    }
    pdf_prior.Read(is);
  }
  SymbolTable words_table(word_file);

  FeaturePipeline feature_pipeline(feature_options);
  OnlineDecodable decodable(tree, pdf_prior, decodable_options,
                            &net, &feature_pipeline);
  FasterDecoder decoder(fst, decoder_options);

  FILE *fp = fopen(wav_scp_file.c_str(), "r");
  if (!fp) {
    ERROR("%s not exint, please check!!!", wav_scp_file.c_str());
  }

  char buffer[1024] = {0}, key[1024] = {0}, path[1024] = {0};
  while (fgets(buffer, 1024, fp)) {
    int num = sscanf(buffer, "%s %s", key, path);
    if (num != 2) {
      ERROR("each line shoud have 2 fields, key & wav path");
    }
    std::cout << key << " " << path << "\n";
    WavReader wav_reader(path);
    CHECK(wav_reader.NumChannel() == 1);
    CHECK(wav_reader.SampleRate() == 16000);
    std::vector<float> wav_data(wav_reader.Data(),
                                wav_reader.Data() + wav_reader.NumSample());
    feature_pipeline.AcceptRawWav(wav_data);
    feature_pipeline.SetDone();

    std::vector<int32_t> result;
    decoder.Decode(&decodable);
    decoder.GetBestPath(&result);

    for (size_t i = 0; i < result.size(); i++) {
      std::cout << words_table.GetSymbol(result[i]) << " ";
    }
    std::cout << "\n";

    // Reset all
    feature_pipeline.Reset();
    decodable.Reset();
  }

  fclose(fp);

  return 0;
}

