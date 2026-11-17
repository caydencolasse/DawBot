import test from 'node:test';
import assert from 'node:assert/strict';

const request = { question: 'How do I freeze a track?', daw: 'Ableton Live 12',
  context: 'SESSION CONTEXT\nDAW: Ableton Live 12', history: [] };
const reply = { answer: 'Freeze the track.', steps: [], midi: null, plugin_chain: [], needs_web: false };
function response(data = reply, annotations = [{ type: 'file_citation', file_id: 'file-1', filename: 'Live12Manual.pdf', index: 0 }], search = 'file_search_call') {
  return { status: 'completed', output: [{ type: search, status: 'completed', results: [] },
    { type: 'message', role: 'assistant', content: [{ type: 'output_text', text: JSON.stringify(data), annotations }] }] };
}
async function run(replies, input = request) {
  const { askCoach } = await import('./coach.mjs');
  const calls = [];
  const result = await askCoach(input, { apiKey: 'server-secret', fetchImpl: async (url, options) => {
    calls.push({ url, ...options, body: JSON.parse(options.body) });
    return new Response(JSON.stringify(replies.shift()), { status: 200 });
  } });
  return { result, calls };
}
test('PDF question uses the configured store, DAW/version, and real file citation without web', async () => {
  const { result, calls } = await run([response()]);
  assert.equal(calls.length, 1);
  assert.equal(calls[0].url, 'https://api.openai.com/v1/responses');
  assert.equal(calls[0].headers.authorization, 'Bearer server-secret');
  assert.equal(calls[0].body.tools[0].vector_store_ids[0], 'vs_6afb8de47e788191b4de19291ee3d6ea');
  assert.deepEqual(calls[0].body.tool_choice, { type: 'file_search' });
  assert.match(JSON.stringify(calls[0].body.input), /Ableton Live 12/);
  assert.match(result.answer, /Manual source: Live12Manual.pdf/);
  assert.doesNotMatch(JSON.stringify(result), /server-secret/);
});
test('insufficient manuals trigger web search and label web sources', async () => {
  const { result, calls } = await run([response({ ...reply, needs_web: true }, []),
    response({ ...reply, answer: 'Current release information.' }, [{ type: 'url_citation', title: 'Release notes', url: 'https://www.ableton.com/release-notes/', start_index: 0, end_index: 5 }], 'web_search_call')]);
  assert.equal(calls.length, 2);
  assert.deepEqual(calls[1].body.tool_choice, { type: 'web_search' });
  assert.match(result.answer, /^Web-sourced answer/);
  assert.match(result.answer, /https:\/\/www.ableton.com\/release-notes\//);
});
test('uncited manual output falls back rather than claiming manual grounding', async () => {
  const { result, calls } = await run([response(reply, []), response(reply, [], 'web_search_call')]);
  assert.equal(calls.length, 2);
  assert.match(result.answer, /could not verify/i);
});
test('refusal and incomplete output are errors', async () => {
  await assert.rejects(run([{ status: 'incomplete', output: [] }]), /incomplete/);
  await assert.rejects(run([{ status: 'completed', output: [{ type: 'message', content: [{ type: 'refusal', refusal: 'Cannot help.' }] }] }]), /declined/);
});
test('missing key and invalid input fail before contacting OpenAI', async () => {
  const { askCoach } = await import('./coach.mjs');
  await assert.rejects(askCoach(request, { apiKey: '' }), /OPENAI_API_KEY/);
  await assert.rejects(askCoach({ ...request, question: '' }, { apiKey: 'test' }), /question/);
});
test('provider errors never expose their body or credentials', async () => {
  const { askCoach } = await import('./coach.mjs');
  await assert.rejects(askCoach(request, { apiKey: 'secret-key', fetchImpl: async () => new Response('secret-key private debug body', { status: 401 }) }), error => {
    assert.match(error.message, /HTTP 401/);
    assert.doesNotMatch(error.message, /secret-key|private debug/);
    return true;
  });
});
test('malformed structured output is rejected', async () => {
  await assert.rejects(run([response({ answer: 'missing required fields' })]), /invalid coaching/);
});
test('without a screenshot the server strips invented screen targets', async () => {
  const { result } = await run([response({ ...reply, steps: [{ instruction: 'Click Freeze', target: { x: 10, y: 10, width: 20, height: 20, label: 'Freeze' } }] })]);
  assert.equal(result.steps[0].target, null);
});
