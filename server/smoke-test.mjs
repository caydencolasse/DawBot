// Live checks: run only after configuring the server-side OPENAI_API_KEY.
import assert from 'node:assert/strict';
import { askCoach } from './coach.mjs';

if (!process.env.OPENAI_API_KEY) {
  console.error('Live tests not run: set OPENAI_API_KEY in the server environment first.');
  process.exitCode = 1;
} else {
  const daw = process.env.DAWBOT_TEST_DAW || 'Ableton Live 12';
  const cases = [
    { expected: 'manual', question: process.env.DAWBOT_TEST_MANUAL_QUESTION || 'According to the manual, how do I freeze an audio track, and what does freezing do?' },
    { expected: 'web', question: `As of ${new Date().toISOString().slice(0, 10)}, what is the latest publicly released version of ${daw.replace(/\s+\d.*$/, '')}? Use current official release notes, including its release date.` }
  ];
  for (const { expected, question } of cases) {
    const reply = await askCoach({ question, daw, context: 'No screenshot attached; all targets must be null.', history: [] });
    console.log(JSON.stringify({ question, expected, source_type: reply.source_type, answer: reply.answer, sources: reply.sources }, null, 2));
    assert.equal(reply.source_type, expected, `${expected} question used the wrong source type`);
    assert.ok(reply.sources.length > 0, 'Answer must cite a source');
    if (expected === 'manual') assert.ok(reply.sources.some(s => /\.pdf$/i.test(s.title)), 'Expected a PDF citation');
    if (expected === 'web') assert.match(reply.answer, /^Web-sourced answer/);
    assert.ok(reply.steps.every(s => s.target === null), 'No screenshot means no screen targets');
  }
}
