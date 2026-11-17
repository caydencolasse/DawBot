import test from 'node:test';
import assert from 'node:assert/strict';
import { once } from 'node:events';
import { createProxy } from './proxy.mjs';
import { readFileSync } from 'node:fs';

test('coach endpoint authenticates, rejects malformed JSON and returns safe errors', async t => {
  const server = createProxy({ tokens: new Set(['dawbot-test-token']), coach: async body => {
    if (body.question === 'fail') throw Object.assign(new Error('Server OPENAI_API_KEY is not configured.'), { status: 503 });
    return { answer: body.daw + ': ' + body.question };
  } });
  server.listen(0, '127.0.0.1');
  await once(server, 'listening');
  t.after(() => new Promise(resolve => { server.close(resolve); server.closeAllConnections(); }));
  const url = `http://127.0.0.1:${server.address().port}/v1/coach`;
  const post = (body, token = 'dawbot-test-token') => fetch(url, { method: 'POST', headers: { authorization: `Bearer ${token}` }, body });
  assert.equal((await post('{}', 'invalid')).status, 401);
  assert.equal((await post('{')).status, 400);
  const ok = await post(JSON.stringify({ daw: 'Logic Pro 11', question: 'How do I bounce?' }));
  assert.equal(ok.status, 200);
  assert.equal((await ok.json()).answer, 'Logic Pro 11: How do I bounce?');
  assert.equal((await post('{"question":"fail"}')).status, 503);
});

test('server coaching schema stays compatible with native steps and MIDI', () => {
  const header = readFileSync(new URL('../Source/Helper/CoachPrompt.h', import.meta.url), 'utf8');
  const native = JSON.parse(header.split('R"JSON(')[1].split(')JSON"')[0]);
  const server = JSON.parse(readFileSync(new URL('./coach-schema.json', import.meta.url), 'utf8'));
  assert.deepEqual(server, native);
});
