import { router } from 'expo-router';
import { Plus, Search } from 'lucide-react-native';
import { useState } from 'react';
import { ScrollView, TextInput, View } from 'react-native';
import { Card, Chip, IconButton, Pill, Row, Screen, T } from '../../components/ui';
import { useStore, type Project } from '../../store';
import { colors, fonts } from '../../theme';

const FILTERS = ['All', 'In progress', 'In studio', 'Finished'] as const;

function statusPill(s: Project['status']) {
  if (s === 'synced') return <Pill label="Synced" tone="green" />;
  if (s === 'studio') return <Pill label="In studio" tone="coral" />;
  return <Pill label="On phone" />;
}

export default function Projects() {
  const { projects, ideas } = useStore();
  const [filter, setFilter] = useState<(typeof FILTERS)[number]>('All');
  const [query, setQuery] = useState('');

  const visible = projects.filter((p) => {
    if (query && !p.name.toLowerCase().includes(query.toLowerCase())) return false;
    if (filter === 'In studio') return p.status !== 'phone';
    if (filter === 'In progress') return p.sections.some((s) => !s.sketched);
    if (filter === 'Finished') return p.sections.every((s) => s.sketched);
    return true;
  });

  return (
    <Screen scroll>
      <Row style={{ paddingHorizontal: 20, paddingTop: 8, justifyContent: 'space-between' }}>
        <T v="display">Projects</T>
        <IconButton icon={Plus} label="New project" tone="coral" href="/projects/new" />
      </Row>
      <View style={{ marginHorizontal: 20, marginTop: 16, height: 44, paddingHorizontal: 14, borderRadius: 12, backgroundColor: colors.card, borderWidth: 1, borderColor: colors.line, flexDirection: 'row', alignItems: 'center', gap: 10 }}>
        <Search size={18} color={colors.text3} strokeWidth={1.75} />
        <TextInput accessibilityLabel="Search projects" placeholder="Search projects" placeholderTextColor={colors.text3} value={query} onChangeText={setQuery} style={{ flex: 1, fontFamily: fonts.regular, fontSize: 15, color: colors.text }} />
      </View>
      <ScrollView horizontal showsHorizontalScrollIndicator={false} contentContainerStyle={{ paddingHorizontal: 20, paddingTop: 12, gap: 8 }}>
        {FILTERS.map((f) => (
          <Chip key={f} label={f} on={filter === f} onPress={() => setFilter(f)} small />
        ))}
      </ScrollView>
      <View style={{ paddingHorizontal: 20, paddingTop: 16, gap: 10 }}>
        {visible.map((p) => {
          const done = p.sections.filter((s) => s.sketched).length;
          const count = ideas.filter((i) => i.projectId === p.id).length;
          return (
            <Card key={p.id} onPress={() => router.push(`/projects/${p.id}` as never)} style={{ padding: 16, gap: 12 }}>
              <Row style={{ justifyContent: 'space-between', alignItems: 'flex-start' }}>
                <View style={{ flex: 1 }}>
                  <T v="bodyStrong" style={{ fontSize: 17 }}>{p.name}</T>
                  <T v="muted" style={{ marginTop: 2 }}>{p.daw} · {p.bpm} BPM · {p.key} · {count} {count === 1 ? 'idea' : 'ideas'}</T>
                </View>
                {statusPill(p.status)}
              </Row>
              <Row gap={4}>
                {p.sections.map((s) => (
                  <View key={s.name} style={{ flex: 1, height: 6, borderRadius: 3, backgroundColor: s.sketched ? colors.coral : colors.raised }} />
                ))}
              </Row>
              <T v="muted">Song map · {done} of {p.sections.length} sections</T>
            </Card>
          );
        })}
        {visible.length === 0 ? <T v="sub" style={{ textAlign: 'center', paddingTop: 24 }}>No projects match</T> : null}
      </View>
    </Screen>
  );
}
