#pragma once

#include <cursiv/completion/DefaultCompletionProvider.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/ast.hpp>

namespace xsmp {

// What the deployment languages share. The parser says what may stand at the
// caret; each language writes its own statements on top.
//
// On top of the default's keywords and names, the editor gets whole statements
// as snippets, values that fit the expected type, and path segments read from
// the model. Items sort by `sortText` in tiers: 1000 names, 2000 values and
// members, 3000 snippets, 4000 bare keywords.
class DeploymentCompletionProvider : public cursiv::DefaultCompletionProvider {
 public:
  using DefaultCompletionProvider::DefaultCompletionProvider;
  ::lsp::CompletionList getCompletion(
      const cursiv::Document&, ::lsp::Position,
      const cursiv::CancellationToken& = {}) const override;

  cursiv::CompletionProviderOptions completionOptions() const override {
    return {.triggerCharacters = {"=", ".", "/", ":"},
            .allCommitCharacters = {}};
  }

 protected:
  using Context = cursiv::CompletionContext;
  using Accept = cursiv::CompletionAcceptor;

  // A component and the path it is reached by from the block being written.
  struct RelativeComponent {
    std::string path;
    const cursiv::AstNode* node;
  };

  // Whether a statement of the language may start at the caret.
  [[nodiscard]] virtual bool atStatement(const Context&) const = 0;

  // What the language offers at the start of a statement, in the block open at
  // the caret, null at the top of the document.
  virtual void completeStatement(const Context&, const cursiv::AstNode* block,
                                 Accept) const = 0;

  // The snippets a keyword of a statement writes.
  virtual void completeKeyword(const Context&, const cursiv::AstNode* block,
                               std::string_view keyword, Accept) const = 0;

  void completionFor(const Context&, const cursiv::NextFeature&,
                     Accept) const override;

  void visitReferenceCandidates(
      const cursiv::ReferenceInfo&, const Context&,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

  [[nodiscard]] cursiv::CompletionValueItem createReferenceCompletionItem(
      const cursiv::AstNodeDescription&, const cursiv::ReferenceInfo&,
      const Context&) const override;

  void completionForKeyword(const Context&, const cursiv::NextFeature&,
                            std::string_view keyword, Accept) const override;

  [[nodiscard]] bool continueCompletion(
      const std::vector<::lsp::CompletionItem>&) const override {
    return true;
  }

  // Whether the grammar may write this member of this class at the caret.
  [[nodiscard]] static bool writes(const Context&, const cursiv::AstTypeTag*,
                                   std::string_view property);

  // Whether this feature reads this keyword.
  [[nodiscard]] bool spells(const cursiv::NextFeature&,
                            std::string_view keyword) const;

  // A statement that writes itself, offered where what is typed matches it.
  void offerSnippet(const Context&, Accept, std::string label, std::string body,
                    std::string detail) const;

  // A statement built from the model, such as `flag = false`, offered where
  // what is typed matches its label or its content. False once the answer is
  // full: what is built from the model can be too much to offer in one answer.
  bool offerValue(const Context&, Accept, std::string label,
                  std::string content, std::string detail,
                  const cursiv::AstNode* node = nullptr) const;

  // The names a member of this class may read here.
  [[nodiscard]] std::vector<std::string> names(const Context&,
                                               const cursiv::AstTypeTag*,
                                               std::string_view property) const;

  // `field = value` for each named field of `component`. An item's detail is
  // `detailPrefix` followed by the name of the component's type.
  void offerFieldAssignments(const Context&, Accept,
                             const cursiv::AstNode* component,
                             std::string_view detailPrefix) const;

  // A `call` for each operation of `component` and a `property` assignment for
  // each property. An entry point is a `subscribe` under `globalEvents`, a
  // `trig` otherwise.
  void offerInvocations(const Context&, Accept,
                        const cursiv::AstNode* component,
                        bool globalEvents) const;

  // An `interface link` from each reference of the components to each other
  // component of a type it accepts, with a back reference where that component
  // has one.
  void offerInterfaceLinks(const Context&, Accept,
                           const std::vector<RelativeComponent>&) const;

  // The two ends of a link as snippet text, `from -> to`: a choice of each among
  // the event sources and sinks of the components, or among their output and
  // input fields, and a tab stop on `from` or `to` where there is none.
  [[nodiscard]] static std::string linkEnds(
      const std::vector<RelativeComponent>&, bool events, std::string_view from,
      std::string_view to);

  // The component a block's statements act on: a model instance itself, or
  // what a configuration, a link base or a task targets. An assembly stands for
  // its root model. Null where that does not resolve.
  [[nodiscard]] static const cursiv::AstNode* componentOf(
      const cursiv::AstNode* block);

  // `component` at path `.` then, under `nested`, every instance below it at
  // its dotted path.
  [[nodiscard]] static std::vector<RelativeComponent> relativeComponents(
      const cursiv::AstNode* component, bool nested);

  // The named members of `owner`: what it declares, then what it inherits. A
  // model instance starts with its child instances.
  [[nodiscard]] static std::vector<const NamedElement*> members(
      const cursiv::AstNode* owner);

  // The snippet tab stop `${index:text}`, with `text` escaped.
  [[nodiscard]] static std::string placeholder(unsigned index,
                                               std::string_view text);

  // The snippet choice `${index|a,b|}` of the distinct `values` that are not
  // empty, or a tab stop on `fallback` where there is none.
  [[nodiscard]] static std::string choice(
      unsigned index, const std::vector<std::string>& values,
      std::string_view fallback);

  // What follows `execute`, as snippet text: the task's name and, where its
  // schedule has parameters, `<p = value, ...> at path`.
  [[nodiscard]] static std::string executeText(const Task& task);

 private:
  void completeValue(const Context&, Accept) const;

  // `field = value` for each field of the structure value at the caret.
  void completeStructure(const Context&, Accept) const;

  // `offerValue`, with the content as `insertText` too.
  void offerTyped(const Context&, Accept, std::string label,
                  std::string content, std::string detail,
                  const cursiv::AstNode* node = nullptr) const;
};

}  // namespace xsmp
