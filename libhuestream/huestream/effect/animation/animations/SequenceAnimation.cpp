/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/Point.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>

#include <string>
#include <memory>
#include <utility>

namespace huestream {

    std::string SequenceAnimation::AttributeBookmarks = "bookmarks";

    PROP_IMPL(SequenceAnimation, AnimationListPtr, sequences, Sequences);

    SequenceAnimation::SequenceAnimation(double repeatTimes)
            : TriggerableAnimation(repeatTimes), _sequences(std::make_shared<AnimationList>()), startIndex_(0),
              currentIndex_(-1) {}

    AnimationPtr SequenceAnimation::Clone() {
        auto deepCopy = std::make_shared<SequenceAnimation>(*this);
        auto sequencesCopy = std::make_shared<AnimationList>();
        for (auto item : *_sequences) {
            sequencesCopy->push_back(item->Clone());
        };
        deepCopy->SetSequences(sequencesCopy);
        return deepCopy;
    }

    void SequenceAnimation::UpdateValue(double *value, double positionMs) {
        positionMs -= offset_;

        for (auto i = startIndex_; i < _sequences->size(); ++i) {
            auto length = (*_sequences)[i]->GetTotalLength();

            if ((*_sequences)[i]->GetTypeName() == SequenceAnimation::type) {
                auto trigger = std::static_pointer_cast<TriggerableAnimation>((*_sequences)[i]);
                length += trigger->GetOffset();
            }

            if (positionMs < length) {
                if (static_cast<int>(i) != currentIndex_) {
                    (*_sequences)[i]->Rewind();
                }

                (*_sequences)[i]->SetMarker(positionMs);
                *value = (*_sequences)[i]->GetValue();
                currentIndex_ = static_cast<int>(i);
                return;
            }

            positionMs -= length;
        }
    }

    double SequenceAnimation::GetLengthMs() const {
        double length = 0;

        for (auto i = startIndex_; i < _sequences->size(); ++i) {
            if ((*_sequences)[i]->IsEndless()) return INF;
            length += (*_sequences)[i]->GetTotalLength();
        }

        return length;
    }

    int SequenceAnimation::Append(AnimationPtr animation, std::string bookmark) {
        _sequences->push_back(animation);
        auto index = static_cast<int>(_sequences->size()) - 1;

        if (!bookmark.empty()) {
            bookmarks_[bookmark] = index;
        }

        return index;
    }

    void SequenceAnimation::UpdateSequence(int startIndex, double startValueMarker) {
        offset_ = startValueMarker;
        startIndex_ = static_cast<size_t>(startIndex);
        SetMarker(_marker);
    }

    std::pair<bool, int> SequenceAnimation::get_trigger_index(std::string bookmark) {
        std::pair<bool, int> result(false, 0);

        auto indexIt = bookmarks_.find(bookmark);
        if (indexIt != bookmarks_.end()) {
            result.first = true;
            result.second = indexIt->second;
        }
        return result;
    }


    void SequenceAnimation::Trigger(const std::string& bookmark) {
        auto indexIt = bookmarks_.find(bookmark);
        if (indexIt != bookmarks_.end()) {
            Trigger(indexIt->second);
        }
    }

    void SequenceAnimation::Trigger(int index) {
        UpdateSequence(index, _marker);
    }

    void SequenceAnimation::TriggerOnLevel(const std::string& bookmark) {
        auto indexIt = bookmarks_.find(bookmark);
        if (indexIt != bookmarks_.end()) {
            auto startIndex = indexIt->second;
            auto startValueMarker = _marker - (*_sequences)[static_cast<size_t>(startIndex)]->GetPositionFromValue(GetValue());
            UpdateSequence(startIndex, startValueMarker);
        }
    }

    bool SequenceAnimation::IsPlaying() const {
        auto totalOffset = offset_;

        if ((*_sequences)[startIndex_]->GetTypeName() == SequenceAnimation::type) {
            auto trigger = std::static_pointer_cast<TriggerableAnimation>((*_sequences)[startIndex_]);
            totalOffset += trigger->GetOffset();
        }

        return _marker - totalOffset < GetLengthMs();
    }


    void SequenceAnimation::Rewind() {
        for (auto &animation : *_sequences) {
            animation->Rewind();
        }
        Animation::Rewind();
        startIndex_ = 0;
        offset_ = 0;
        currentIndex_ = -1;
    }

    std::string SequenceAnimation::GetTypeName() const {
        return type;
    }

    void SequenceAnimation::Serialize(JSONNode *node) const {
        RepeatableAnimation::Serialize(node);

        JSONNode arrayNode(JSON_ARRAY);
        arrayNode.set_name(AttributeSequences);
        for (size_t i = 0; i < _sequences->size(); ++i) {
            JSONNode v;
            (*_sequences)[i]->Serialize(&v);
            arrayNode.push_back(v);
        }
        node->push_back(arrayNode);


        JSONNode bookmakersNode;
        for (auto it = bookmarks_.begin(); it != bookmarks_.end(); ++it) {
            auto bookmark = it->first;
            auto index = it->second;
            bookmakersNode.push_back(JSONNode(bookmark, index));
        }
        bookmakersNode.set_name(AttributeBookmarks);
        node->push_back(bookmakersNode);
    }

    void SequenceAnimation::Deserialize(JSONNode *node) {
        RepeatableAnimation::Deserialize(node);

        _sequences->clear();
        if (SerializerHelper::IsAttributeSet(node, AttributeSequences)) {
            auto listNode = (*node)[AttributeSequences];
            for (auto it = listNode.begin(); it != listNode.end(); ++it) {
                JSONNode itemNode = *it;
                auto animation = std::static_pointer_cast<Animation>(Serializable::DeserializeFromJson(&itemNode));
                _sequences->push_back(animation);
            }
        }

        bookmarks_.clear();
        if (SerializerHelper::IsAttributeSet(node, AttributeBookmarks)) {
            auto listNode = (*node)[AttributeBookmarks];
            for (auto it = listNode.begin(); it != listNode.end(); ++it) {
                bookmarks_[it->name()] = static_cast<int>(it->as_int());
            }
        }
    }

    int SequenceAnimation::get_num_sequences() const {
        return static_cast<int>(_sequences->size());
    }

    int SequenceAnimation::get_num_triggers() const {
        return static_cast<int>(bookmarks_.size());
    }


}  // namespace huestream
