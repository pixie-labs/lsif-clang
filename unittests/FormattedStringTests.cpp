//===-- FormattedStringTests.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "FormattedString.h"
#include "clang/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace clang {
namespace clangd {
namespace markup {
namespace {

TEST(Render, Escaping) {
  // Check some ASCII punctuation
  Paragraph P;
  P.appendText("*!`");
  EXPECT_EQ(P.asMarkdown(), "\\*\\!\\`");

  // Check all ASCII punctuation.
  P = Paragraph();
  std::string Punctuation = R"txt(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)txt";
  // Same text, with each character escaped.
  std::string EscapedPunctuation;
  EscapedPunctuation.reserve(2 * Punctuation.size());
  for (char C : Punctuation)
    EscapedPunctuation += std::string("\\") + C;
  P.appendText(Punctuation);
  EXPECT_EQ(P.asMarkdown(), EscapedPunctuation);

  // In code blocks we don't need to escape ASCII punctuation.
  P = Paragraph();
  P.appendCode("* foo !+ bar * baz");
  EXPECT_EQ(P.asMarkdown(), "`* foo !+ bar * baz`");

  // But we have to escape the backticks.
  P = Paragraph();
  P.appendCode("foo`bar`baz");
  EXPECT_EQ(P.asMarkdown(), "`foo``bar``baz`");

  // Inline code blocks starting or ending with backticks should add spaces.
  P = Paragraph();
  P.appendCode("`foo");
  EXPECT_EQ(P.asMarkdown(), "` ``foo `");
  P = Paragraph();
  P.appendCode("foo`");
  EXPECT_EQ(P.asMarkdown(), "` foo`` `");
  P = Paragraph();
  P.appendCode("`foo`");
  EXPECT_EQ(P.asMarkdown(), "` ``foo`` `");
}

TEST(Paragraph, SeparationOfChunks) {
  // This test keeps appending contents to a single Paragraph and checks
  // expected accumulated contents after each one.
  // Purpose is to check for separation between different chunks.
  Paragraph P;

  P.appendText("after");
  EXPECT_EQ(P.asMarkdown(), "after");
  EXPECT_EQ(P.asPlainText(), "after");

  P.appendCode("foobar");
  EXPECT_EQ(P.asMarkdown(), "after `foobar`");
  EXPECT_EQ(P.asPlainText(), "after foobar");

  P.appendText("bat");
  EXPECT_EQ(P.asMarkdown(), "after `foobar` bat");
  EXPECT_EQ(P.asPlainText(), "after foobar bat");
}

TEST(Paragraph, ExtraSpaces) {
  // Make sure spaces inside chunks are dropped.
  Paragraph P;
  P.appendText("foo\n   \t   baz");
  P.appendCode(" bar\n");
  EXPECT_EQ(P.asMarkdown(), R"md(foo baz `bar`)md");
  EXPECT_EQ(P.asPlainText(), R"pt(foo baz bar)pt");
}

TEST(Paragraph, NewLines) {
  // New lines before and after chunks are dropped.
  Paragraph P;
  P.appendText(" \n foo\nbar\n ");
  P.appendCode(" \n foo\nbar \n ");
  EXPECT_EQ(P.asMarkdown(), R"md(foo bar `foo bar`)md");
  EXPECT_EQ(P.asPlainText(), R"pt(foo bar foo bar)pt");
}

TEST(Document, Separators) {
  Document D;
  D.addParagraph().appendText("foo");
  D.addParagraph().appendText("bar");
  EXPECT_EQ(D.asMarkdown(), "foo\nbar");
  EXPECT_EQ(D.asPlainText(), "foo\nbar");
}

TEST(Document, Spacer) {
  Document D;
  D.addParagraph().appendText("foo");
  D.addSpacer();
  D.addParagraph().appendText("bar");
  EXPECT_EQ(D.asMarkdown(), "foo\n\nbar");
  EXPECT_EQ(D.asPlainText(), "foo\n\nbar");
}

} // namespace
} // namespace markup
} // namespace clangd
} // namespace clang
