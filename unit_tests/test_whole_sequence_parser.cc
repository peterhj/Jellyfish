#include <gtest/gtest.h>
#include <jflib/tmpstream.hpp>
#include <jellyfish/whole_sequence_parser.hpp>

namespace {
using std::string;
using std::istringstream;
template<typename Iterator>
struct opened_streams {
  Iterator begin_, end_;

  opened_streams(Iterator begin, Iterator end) : begin_(begin), end_(end) { }
  std::unique_ptr<std::istream> next() {
    std::unique_ptr<std::istream> res;
    if(begin_ != end_) {
      res.reset(*begin_);
      ++begin_;
    }
    return res;
  }
};
typedef jellyfish::whole_sequence_parser<opened_streams<jflib::tmpstream**> > parser_type;

TEST(SequenceParser, Fasta) {
  static const char* seq1 = "ATTACCTTGTACCTTCAGAGC";
  static const char* seq2 = "TTCGATCCCTTGATAATTAGTCACGTTAGCT";
  jflib::tmpstream* sequence = new jflib::tmpstream;
  *sequence << ">header1\n"
           << seq1 << "\n\n"
           << ">header2\n"
           << seq2 << "\n\n\n"
           << seq1;
  sequence->seekg(0);

  opened_streams<jflib::tmpstream**> streams(&sequence, &sequence + 1);
  parser_type parser(10, 1, streams);

  {
    parser_type::job j(parser);
    ASSERT_FALSE(j.is_empty());
    EXPECT_STREQ("header1", j->header.c_str());
    EXPECT_STREQ(seq1, j->seq.c_str());
  }

  {
    parser_type::job j(parser);
    ASSERT_FALSE(j.is_empty());
    EXPECT_STREQ("header2", j->header.c_str());
    EXPECT_EQ(strlen(seq2) + strlen(seq1), j->seq.size());
    EXPECT_STREQ(seq2, j->seq.substr(0, strlen(seq2)).c_str());
    EXPECT_STREQ(seq1, j->seq.substr(strlen(seq2)).c_str());
  }

  {
    parser_type::job j(parser);
    EXPECT_TRUE(j.is_empty());
  }
}

TEST(SequenceParser, Fastq) {
  static const char* seq1 = "ATTACCTTGTACCTTCAGAGC";
  static const char* seq2 = "TTCGATCCCTTGATAATTAGTCACGTTAGCT";
  jflib::tmpstream* sequence = new jflib::tmpstream;
  *sequence << "@header1\n"
           << seq1 << "\n\n"
           << "+and some stuff\n"
           << seq1 << "\n"
           << "@header2\n"
           << seq2 << "\n\n\n"
           << seq1 << "\n"
           << "+\n"
           << seq1 << "\n"
           << seq2 << "\n";
  sequence->seekg(0);

  opened_streams<jflib::tmpstream**> streams(&sequence, &sequence + 1);
  parser_type parser(10, 1, streams);

  {
    parser_type::job j(parser);
    ASSERT_FALSE(j.is_empty());
    EXPECT_STREQ("header1", j->header.c_str());
    EXPECT_STREQ(seq1, j->seq.c_str());
    EXPECT_STREQ(seq1, j->qual.c_str());
  }

  {
    parser_type::job j(parser);
    ASSERT_FALSE(j.is_empty());
    EXPECT_STREQ("header2", j->header.c_str());
    EXPECT_EQ(strlen(seq2) + strlen(seq1), j->seq.size());
    EXPECT_EQ(strlen(seq2) + strlen(seq1), j->qual.size());
    EXPECT_STREQ(seq2, j->seq.substr(0, strlen(seq2)).c_str());
    EXPECT_STREQ(seq1, j->seq.substr(strlen(seq2)).c_str());
    EXPECT_STREQ(seq1, j->qual.substr(0, strlen(seq1)).c_str());
    EXPECT_STREQ(seq2, j->qual.substr(strlen(seq1)).c_str());
  }

  {
    parser_type::job j(parser);
    EXPECT_TRUE(j.is_empty());
  }
}

}