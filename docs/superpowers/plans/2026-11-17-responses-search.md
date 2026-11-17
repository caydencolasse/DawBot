# Responses search integration plan

Goal: connect the existing native chat to a server-only Responses API using vector store `vs_6afb8de47e788191b4de19291ee3d6ea`, with manual-first answers and labelled web fallback.

Approach: retain the Node proxy and JUCE chat/steps/MIDI contract. Add `/v1/coach`; force file search on the first Responses request, then allow only web search if evidence is insufficient. Keep the existing combined DAW/version preference intact and send it on every question. Add source names/URLs to displayed answer text. Do not build a new browser app or change Anthropic behavior.

- [x] Add server regression tests for grounded PDF replies, web fallback, context forwarding, missing keys, malformed responses and authentication.
- [x] Implement the endpoint and server-owned schema/tool configuration with bounded requests and safe errors.
- [x] Connect the OpenAI Helper path and prevent local OpenAI key storage/use; expose DawBot token configuration.
- [ ] Run Node tests, attempt native build/regression checks, and run one PDF and one current-information live question if a server key is available.
- [ ] Review the diff, document setup and verification limits, commit, push and open a PR.

Review focus: no fabricated citations; no web answer represented as a manual answer; no DAW/version context lost; no API key in client payload/config; refusal, partial output and upstream errors must surface as errors.

Verification note: user will configure the OpenAI key later, so live PDF/web calls are deferred. `server/smoke-test.mjs` is ready for that step.
