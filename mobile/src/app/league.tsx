import { ArrowUp } from 'lucide-react-native';
import { Fragment } from 'react';
import { Text, View } from 'react-native';
import { NavHeader, Row, Screen, T } from '../components/ui';
import { useStore } from '../store';
import { colors, fonts } from '../theme';

// TODO(api): replace with the weekly league from the dawbot backend.
const PLAYERS = [
  { name: 'nova.beats', xp: 512 },
  { name: 'Lio M.', xp: 488 },
  { name: 'kaimakes', xp: 401 },
  { name: 'You', xp: 0 },
  { name: 'J. Okafor', xp: 322 },
  { name: 'saturn_tapes', xp: 297 },
  { name: 'Mira', xp: 260 },
  { name: 'deeplowend', xp: 233 },
  { name: 'Theo R.', xp: 198 },
];
const PROMOTE = 7;

const initials = (n: string) =>
  n.replace(/[._]/g, ' ').split(' ').filter(Boolean).slice(0, 2).map((w) => w[0]).join('').toUpperCase();

export default function League() {
  const { xp } = useStore();
  const rows = PLAYERS.map((p) => (p.name === 'You' ? { ...p, xp } : p)).sort((a, b) => b.xp - a.xp);
  return (
    <Screen scroll>
      <NavHeader title="Bronze league" />
      <T v="sub" style={{ paddingHorizontal: 20 }}>
        Earn XP from lessons and ideas. The week resets in <Text style={{ fontFamily: fonts.semibold, color: colors.text }}>3 days</Text>
      </T>
      <View style={{ paddingHorizontal: 12, paddingTop: 20, gap: 4 }}>
        {rows.map((p, i) => {
          const you = p.name === 'You';
          return (
            <Fragment key={p.name}>
              <Row style={{ height: 56, paddingHorizontal: 12, borderRadius: 12, borderWidth: 1, borderColor: you ? colors.coral : 'transparent', backgroundColor: you ? colors.coralBg : 'transparent' }}>
                <Text style={{ width: 20, fontFamily: fonts.semibold, fontSize: 15, color: i < 3 ? colors.coral : colors.text3, fontVariant: ['tabular-nums'] }}>{i + 1}</Text>
                <View style={{ width: 36, height: 36, borderRadius: 18, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
                  <Text style={{ fontFamily: fonts.semibold, fontSize: 13, color: colors.text }}>{initials(p.name)}</Text>
                </View>
                <Text style={{ flex: 1, fontFamily: you ? fonts.semibold : fonts.regular, fontSize: 15, color: colors.text }}>{p.name}</Text>
                <T v="small" style={{ fontVariant: ['tabular-nums'] }}>{p.xp} XP</T>
              </Row>
              {i === PROMOTE - 1 ? (
                <Row gap={8} style={{ paddingHorizontal: 12, paddingVertical: 8 }}>
                  <ArrowUp size={14} color={colors.green} strokeWidth={2} />
                  <Text style={{ fontFamily: fonts.medium, fontSize: 13, color: colors.green }}>Top {PROMOTE} move up to Silver</Text>
                </Row>
              ) : null}
            </Fragment>
          );
        })}
      </View>
    </Screen>
  );
}
