# Clean-Room Synthesis Methodology

## Purpose

This document defines the clean-room process for libfixp development. The goal is to create an original implementation informed by public knowledge without copying existing proprietary or copyleft code.

## Legal Context

### Clean-Room Design
A development practice where:
1. One team/phase studies existing implementations (research)
2. Another team/phase implements from specifications only (synthesis)
3. No direct code copying or derivative work

### Why It Matters
- Avoids copyright infringement claims
- Ensures true intellectual ownership
- Enables permissive licensing
- Demonstrates original authorship

## Our Process

### Phase 1: Research (Current)

**Activities:**
- Study academic papers and textbooks
- Analyze existing library documentation (not source)
- Review standards documents (ISO, ARM ACLE, etc.)
- Examine public API designs
- Document CONCEPTS, ALGORITHMS, and THEORY

**Outputs:**
- `research/concepts/` - Algorithm descriptions, theory
- `research/standards/` - Standards analysis
- `research/papers/` - Paper summaries and citations
- `research/implementations/` - WHAT libraries do, not HOW

**Rules:**
- NO source code in research documents
- Document in natural language and math notation
- Focus on "what problem is solved" and "what approach is used"
- Never transcribe implementation details

### Phase 2: Specification (Next)

**Activities:**
- Define libfixp API based on research concepts
- Write formal specifications for each function
- Design type system and templates
- Specify accuracy requirements
- Define test vectors (from mathematical derivation, not other tests)

**Outputs:**
- `docs/spec/` - Formal API specification
- `docs/spec/accuracy.md` - Precision requirements
- `docs/spec/conformance.md` - Test requirements

**Rules:**
- Specifications are mathematical, not code
- No reference to specific implementations
- Test vectors derived from first principles

### Phase 3: Implementation (Future)

**Activities:**
- Implement from specifications ONLY
- No looking back at research during coding
- Derive algorithms from mathematics
- Verify against mathematical test vectors

**Outputs:**
- `src/` - All source code

**Rules:**
- Team members who implement must not have seen source code
- If same person does both, enforce temporal separation
- Document implementation decisions independently
- All optimizations derived from first principles

## Information Barriers

### Physical Separation
- Research documents in `research/`
- Implementation in `src/`
- Never mix in same session

### Temporal Separation
- Complete research phase before implementation
- "Cooling off" period between phases
- Fresh perspective when coding

### Documentation Discipline
- Research: "The algorithm computes X by iteratively refining Y"
- Implementation: Derive HOW to iterate from scratch

## Concept Extraction Rules

### Allowed
- Mathematical formulas (e.g., CORDIC rotation equations)
- Algorithm complexity analysis
- API design patterns (function names, signatures)
- Published benchmark results
- Standards requirements
- Academic paper contents

### Not Allowed
- Copying code snippets
- Translating code to pseudocode
- Detailed implementation tricks (unless published in papers)
- Test vectors from other implementations
- Build system configurations

## Documentation Standards

### Concept Documents Should Contain
- Natural language descriptions
- Mathematical notation
- Block diagrams
- Complexity analysis
- Trade-off discussions
- References to papers/standards

### Concept Documents Must NOT Contain
- Source code in any language
- Pseudocode that could be directly translated
- Specific variable names from other implementations
- Memory layouts copied from implementations

## Verification

### Pre-Implementation Checklist
- [ ] All concept documents reviewed for code contamination
- [ ] No source code files in research/
- [ ] Specifications are mathematically complete
- [ ] Test vectors derived from math, not borrowed

### Post-Implementation Checklist
- [ ] No unusual similarity to existing implementations
- [ ] Can explain every algorithm from first principles
- [ ] Documentation trace shows independent derivation

## Example: CORDIC Implementation

### Research Phase Documentation (Correct)
> "CORDIC computes trigonometric functions by rotating a vector through
> pre-computed angles arctan(2^-i). In rotation mode, the angle Z is
> driven to zero, leaving the sine in Y and cosine in X, scaled by
> factor K = product(sqrt(1 + 2^-2i))."

### Research Phase Documentation (Incorrect - Too Specific)
> "The loop uses variables x, y, z. Each iteration:
> x_new = x - sigma * y >> i;
> y_new = y + sigma * x >> i;"
> (This is pseudocode, not allowed)

### Implementation Phase (Independently Derived)
- Start with rotation matrix math
- Derive iterative form from factorization
- Choose variable names independently
- Implement and verify against mathematical values

## Team Simulation (Solo Developer)

For single-developer projects:
1. **Time separation**: Complete all research before any implementation
2. **Document separation**: Close all research files when coding
3. **Mindset shift**: Pretend you're a different person
4. **Fresh derivation**: Re-derive algorithms from theory when implementing
5. **Review gates**: Self-audit for contamination

## References

- Clean-room design legal precedent
- Phoenix Technologies BIOS development model
- Academic integrity principles
