// DawBot API proxy (development version). Node 18+, no dependencies.
//
// Why: the Helper app should never ship with your Anthropic/OpenAI keys inside it.
// In production the Helper points its Base URL at https://api.dawbot.ai and sends the
// user's DawBot token; this proxy checks the token, forwards the request with YOUR
// key, and records token usage so you can bill credits.
//
// Run:  ANTHROPIC_API_KEY=... OPENAI_API_KEY=... DAWBOT_DEV_TOKENS=devtoken123 node server/proxy.mjs
// Then in DawBot Helper's config.json set: "baseUrl": "http://localhost:8787", "authToken": "devtoken123"

import http from 'node:http';

const PORT = Number(process.env.PORT || 8787);
const MAX_BODY = 8 * 1024 * 1024; // screenshots are ~150-400 KB as JPEG
const TOKENS = new Set((process.env.DAWBOT_DEV_TOKENS || '').split(',').map(s => s.trim()).filter(Boolean));

const ROUTES = {
  '/v1/messages': {
    url: 'https://api.anthropic.com/v1/messages',
    headers: () => ({ 'x-api-key': process.env.ANTHROPIC_API_KEY, 'anthropic-version': '2023-06-01' }),
  },
  '/v1/chat/completions': {
    url: 'https://api.openai.com/v1/chat/completions',
    headers: () => ({ authorization: `Bearer ${process.env.OPENAI_API_KEY}` }),
  },
};

// token -> { input, output, requests }. Replace with your database + credit ledger.
const usage = new Map();

function send(res, status, obj) {
  res.writeHead(status, { 'content-type': 'application/json' });
  res.end(JSON.stringify(obj));
}

http.createServer(async (req, res) => {
  const route = ROUTES[req.url];
  if (req.method !== 'POST' || !route) return send(res, 404, { error: { message: 'Not found' } });

  const token = (req.headers.authorization || '').replace(/^Bearer\s+/i, '');
  if (!TOKENS.has(token)) return send(res, 401, { error: { message: 'Invalid or expired DawBot token' } });

  // TODO(production): look up the user's subscription/credit balance here and reject with 402 when empty.

  const chunks = [];
  let size = 0;
  for await (const chunk of req) {
    size += chunk.length;
    if (size > MAX_BODY) return send(res, 413, { error: { message: 'Request too large' } });
    chunks.push(chunk);
  }

  try {
    const upstream = await fetch(route.url, {
      method: 'POST',
      headers: { 'content-type': 'application/json', ...route.headers() },
      body: Buffer.concat(chunks),
    });
    const text = await upstream.text();

    try {
      const u = JSON.parse(text).usage;
      if (u) {
        const entry = usage.get(token) || { input: 0, output: 0, requests: 0 };
        entry.input += u.input_tokens ?? u.prompt_tokens ?? 0;
        entry.output += u.output_tokens ?? u.completion_tokens ?? 0;
        entry.requests += 1;
        usage.set(token, entry);
        console.log(`[usage] ${token.slice(0, 6)}… ${JSON.stringify(entry)}`);
      }
    } catch { /* non-JSON error body: pass through */ }

    res.writeHead(upstream.status, { 'content-type': 'application/json' });
    res.end(text);
  } catch (err) {
    send(res, 502, { error: { message: `Upstream error: ${err}` } });
  }
}).listen(PORT, () => console.log(`DawBot proxy listening on http://localhost:${PORT}`));
