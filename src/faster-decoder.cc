// decoder/faster-decoder.cc

// Copyright 2009-2011 Microsoft Corporation
//           2012-2013 Johns Hopkins University (author: Daniel Povey)
//           2018 Personal (Binbin Zhang)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>

#include "faster-decoder.h"

namespace xdecoder {


FasterDecoder::FasterDecoder(const Fst& fst,
                             const FasterDecoderOptions& opts):
    fst_(fst), config_(opts), num_frames_decoded_(-1) {
  CHECK(config_.hash_ratio >= 1.0);  // less doesn't make much sense.
  CHECK(config_.max_active > 1);
  CHECK(config_.min_active >= 0 && config_.min_active < config_.max_active);
  // just so on the first frame we do something reasonable.
  toks_.SetSize(1000);
}

void FasterDecoder::InitDecoding() {
  // clean up from last time:
  ClearToks(toks_.Clear());
  int32_t start_state = fst_.Start();
  Arc dummy_arc(0, 0, 0.0f, start_state);
  toks_.Insert(start_state, new Token(dummy_arc, NULL));
  ProcessNonemitting(std::numeric_limits<float>::max());
  num_frames_decoded_ = 0;
}

void FasterDecoder::Decode(Decodable* decodable) {
  InitDecoding();
  while (!decodable->IsLastFrame(num_frames_decoded_ - 1)) {
    double weight_cutoff = ProcessEmitting(decodable);
    ProcessNonemitting(weight_cutoff);
  }
}

void FasterDecoder::AdvanceDecoding(Decodable *decodable,
                                      int32_t max_num_frames) {
  CHECK(num_frames_decoded_ >= 0 &&
               "You must call InitDecoding() before AdvanceDecoding()");
  int32_t num_frames_ready = decodable->NumFramesReady();
  // num_frames_ready must be >= num_frames_decoded, or else
  // the number of frames ready must have decreased (which doesn't
  // make sense) or the decodable object changed between calls
  // (which isn't allowed).
  CHECK(num_frames_ready >= num_frames_decoded_);
  int32_t target_frames_decoded = num_frames_ready;
  if (max_num_frames >= 0)
    target_frames_decoded = std::min(target_frames_decoded,
                                     num_frames_decoded_ + max_num_frames);
  while (num_frames_decoded_ < target_frames_decoded) {
    // note: ProcessEmitting() increments num_frames_decoded_
    double weight_cutoff = ProcessEmitting(decodable);
    ProcessNonemitting(weight_cutoff);
  }
}

bool FasterDecoder::ReachedFinal() {
  for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
    if (e->val->cost_ != std::numeric_limits<double>::infinity() &&
        fst_.IsFinal(e->key))
      return true;
  }
  return false;
}

bool FasterDecoder::GetBestPath(std::vector<int32_t> *results,
                                bool use_final_probs) {
  // GetBestPath gets the decoding output.  If "use_final_probs" is true
  // AND we reached a final state, it limits itself to final states;
  // otherwise it gets the most likely token not taking into
  // account final-probs.  results will be empty if
  // nothing was available.  It returns true if it got output.
  results->clear();
  Token *best_tok = NULL;
  bool is_final = ReachedFinal();
  if (!is_final) {
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
      if (best_tok == NULL || *best_tok < *(e->val) )
        best_tok = e->val;
  } else {
    double infinity =  std::numeric_limits<double>::infinity(),
        best_cost = infinity;
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
      double this_cost = e->val->cost_ + fst_.Final(e->key);
      if (this_cost < best_cost && this_cost != infinity) {
        best_cost = this_cost;
        best_tok = e->val;
      }
    }
  }
  if (best_tok == NULL) return false;  // No output.

  std::vector<int32_t> results_reverse;
  for (Token *tok = best_tok; tok != NULL; tok = tok->prev_) {
    results_reverse.push_back(tok->arc_.olabel);
  }
  results_reverse.pop_back();  // that was a "fake" token... gives no info.

  std::vector<int32_t>::iterator it =
      std::unique(results_reverse.begin(), results_reverse.end());
  results->insert(results->begin(), results_reverse.begin(), it);
  std::reverse(results->begin(), results->end());
  return true;
}

// Gets the weight cutoff.  Also counts the active tokens.
double FasterDecoder::GetCutoff(Elem *list_head, size_t *tok_count,
                                float *adaptive_beam, Elem **best_elem) {
  double best_cost = std::numeric_limits<double>::infinity();
  size_t count = 0;
  if (config_.max_active == std::numeric_limits<int32_t>::max() &&
      config_.min_active == 0) {
    for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
      double w = e->val->cost_;
      if (w < best_cost) {
        best_cost = w;
        if (best_elem) *best_elem = e;
      }
    }
    if (tok_count != NULL) *tok_count = count;
    if (adaptive_beam != NULL) *adaptive_beam = config_.beam;
    return best_cost + config_.beam;
  } else {
    tmp_array_.clear();
    for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
      double w = e->val->cost_;
      tmp_array_.push_back(w);
      if (w < best_cost) {
        best_cost = w;
        if (best_elem) *best_elem = e;
      }
    }
    if (tok_count != NULL) *tok_count = count;
    double beam_cutoff = best_cost + config_.beam,
        min_active_cutoff = std::numeric_limits<double>::infinity(),
        max_active_cutoff = std::numeric_limits<double>::infinity();

    if (tmp_array_.size() > static_cast<size_t>(config_.max_active)) {
      std::nth_element(tmp_array_.begin(),
                       tmp_array_.begin() + config_.max_active,
                       tmp_array_.end());
      max_active_cutoff = tmp_array_[config_.max_active];
    }
    if (max_active_cutoff < beam_cutoff) {  // max_active is tighter than beam.
      if (adaptive_beam)
        *adaptive_beam = max_active_cutoff - best_cost + config_.beam_delta;
      return max_active_cutoff;
    }
    if (tmp_array_.size() > static_cast<size_t>(config_.min_active)) {
      if (config_.min_active == 0) {
        min_active_cutoff = best_cost;
      } else {
        std::nth_element(tmp_array_.begin(),
            tmp_array_.begin() + config_.min_active,
            tmp_array_.size() > static_cast<size_t>(config_.max_active) ?
            tmp_array_.begin() + config_.max_active :
            tmp_array_.end());
        min_active_cutoff = tmp_array_[config_.min_active];
      }
    }
    if (min_active_cutoff > beam_cutoff) {  // min_active is looser than beam.
      if (adaptive_beam)
        *adaptive_beam = min_active_cutoff - best_cost + config_.beam_delta;
      return min_active_cutoff;
    } else {
      *adaptive_beam = config_.beam;
      return beam_cutoff;
    }
  }
}

void FasterDecoder::PossiblyResizeHash(size_t num_toks) {
  size_t new_sz = static_cast<size_t>(static_cast<float>(num_toks)
                                      * config_.hash_ratio);
  if (new_sz > toks_.Size()) {
    toks_.SetSize(new_sz);
  }
}

// ProcessEmitting returns the likelihood cutoff used.
double FasterDecoder::ProcessEmitting(Decodable* decodable) {
  int32_t frame = num_frames_decoded_;
  Elem *last_toks = toks_.Clear();
  size_t tok_cnt;
  float adaptive_beam;
  Elem *best_elem = NULL;
  double weight_cutoff = GetCutoff(last_toks, &tok_cnt,
                                   &adaptive_beam, &best_elem);
  // KALDI_VLOG(3) << tok_cnt << " tokens active.";
  PossiblyResizeHash(tok_cnt);  // This makes sure the hash is always big enough

  // This is the cutoff we use after adding in the log-likes (i.e.
  // for the next frame).  This is a bound on the cutoff we will use
  // on the next frame.
  double next_weight_cutoff = std::numeric_limits<double>::infinity();

  // First process the best token to get a hopefully
  // reasonably tight bound on the next cutoff.
  if (best_elem) {
    int32_t state = best_elem->key;
    Token *tok = best_elem->val;
    for (const Arc* it = fst_.ArcStart(state); it != fst_.ArcEnd(state); it++) {
      const Arc &arc = *it;
      if (arc.ilabel != 0) {  // we'd propagate..
        float ac_cost = - decodable->LogLikelihood(frame, arc.ilabel);
        double new_weight = arc.weight + tok->cost_ + ac_cost;
        if (new_weight + adaptive_beam < next_weight_cutoff)
          next_weight_cutoff = new_weight + adaptive_beam;
      }
    }
  }

  // int32_t n = 0, np = 0;

  // the tokens are now owned here, in last_toks, and the hash is empty.
  // 'owned' is a complex thing here; the point is we need to call TokenDelete
  // on each elem 'e' to let toks_ know we're done with them.
  for (Elem *e = last_toks, *e_tail; e != NULL; e = e_tail) {  // loop this way
    // n++;
    // because we delete "e" as we go.
    int32_t state = e->key;
    Token *tok = e->val;
    if (tok->cost_ < weight_cutoff) {  // not pruned.
      // np++;
      CHECK(state == tok->arc_.next_state);
      for (const Arc* it = fst_.ArcStart(state);
           it != fst_.ArcEnd(state); it++) {
        const Arc &arc = *it;
        if (arc.ilabel != 0) {  // propagate..
          float ac_cost =  - decodable->LogLikelihood(frame, arc.ilabel);
          double new_weight = arc.weight + tok->cost_ + ac_cost;
          if (new_weight < next_weight_cutoff) {  // not pruned..
            Token *new_tok = new Token(arc, ac_cost, tok);
            Elem *e_found = toks_.Find(arc.next_state);
            if (new_weight + adaptive_beam < next_weight_cutoff)
              next_weight_cutoff = new_weight + adaptive_beam;
            if (e_found == NULL) {
              toks_.Insert(arc.next_state, new_tok);
            } else {
              if ( *(e_found->val) < *new_tok ) {
                Token::TokenDelete(e_found->val);
                e_found->val = new_tok;
              } else {
                Token::TokenDelete(new_tok);
              }
            }
          }
        }
      }
    }
    e_tail = e->tail;
    Token::TokenDelete(e->val);
    toks_.Delete(e);
  }
  num_frames_decoded_++;
  return next_weight_cutoff;
}

// TODO(Binbin): first time we go through this, could avoid using the queue.
void FasterDecoder::ProcessNonemitting(double cutoff) {
  // Processes nonemitting arcs for one frame.
  CHECK(queue_.empty());
  for (const Elem *e = toks_.GetList(); e != NULL;  e = e->tail)
    queue_.push_back(e->key);
  while (!queue_.empty()) {
    int32_t state = queue_.back();
    queue_.pop_back();
    Token *tok = toks_.Find(state)->val;  // would segfault if state not
    // in toks_ but this can't happen.
    if (tok->cost_ > cutoff) {  // Don't bother processing successors.
      continue;
    }
    CHECK(tok != NULL && state == tok->arc_.next_state);
    for (const Arc *it = fst_.ArcStart(state);
        it != fst_.ArcEnd(state); it++) {
      const Arc &arc = *it;
      if (arc.ilabel == 0) {  // propagate nonemitting only...
        Token *new_tok = new Token(arc, tok);
        if (new_tok->cost_ > cutoff) {  // prune
          Token::TokenDelete(new_tok);
        } else {
          Elem *e_found = toks_.Find(arc.next_state);
          if (e_found == NULL) {
            toks_.Insert(arc.next_state, new_tok);
            queue_.push_back(arc.next_state);
          } else {
            if ( *(e_found->val) < *new_tok ) {
              Token::TokenDelete(e_found->val);
              e_found->val = new_tok;
              queue_.push_back(arc.next_state);
            } else {
              Token::TokenDelete(new_tok);
            }
          }
        }
      }
    }
  }
}

void FasterDecoder::ClearToks(Elem *list) {
  for (Elem *e = list, *e_tail; e != NULL; e = e_tail) {
    Token::TokenDelete(e->val);
    e_tail = e->tail;
    toks_.Delete(e);
  }
}

}  // namespace xdecoder
