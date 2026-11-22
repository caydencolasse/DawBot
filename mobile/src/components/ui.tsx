import { router } from 'expo-router';
import { Check, ChevronLeft, Flame, X, type LucideIcon } from 'lucide-react-native';
import type { ReactNode } from 'react';
import {
  Image,
  Pressable,
  ScrollView,
  StyleSheet,
  Text,
  View,
  type StyleProp,
  type TextStyle,
  type ViewStyle,
} from 'react-native';
import { SafeAreaView, useSafeAreaInsets } from 'react-native-safe-area-context';
import { dawById, type DawId } from '../assets';
import { colors, fonts, radius, type } from '../theme';

// ---------------------------------------------------------------- layout

export function Screen({
  children,
  scroll = false,
  footer,
  edges = ['top'],
  contentStyle,
}: {
  children: ReactNode;
  scroll?: boolean;
  footer?: ReactNode;
  edges?: ('top' | 'bottom')[];
  contentStyle?: StyleProp<ViewStyle>;
}) {
  const insets = useSafeAreaInsets();
  return (
    <SafeAreaView style={styles.screen} edges={edges}>
      {scroll ? (
        <ScrollView contentContainerStyle={[{ paddingBottom: 24 }, contentStyle]} showsVerticalScrollIndicator={false}>
          {children}
        </ScrollView>
      ) : (
        <View style={[{ flex: 1 }, contentStyle]}>{children}</View>
      )}
      {footer ? <View style={[styles.footer, { paddingBottom: Math.max(insets.bottom, 16) + 8 }]}>{footer}</View> : null}
    </SafeAreaView>
  );
}

export function Row({ children, gap = 12, style }: { children: ReactNode; gap?: number; style?: StyleProp<ViewStyle> }) {
  return <View style={[{ flexDirection: 'row', alignItems: 'center', gap }, style]}>{children}</View>;
}

// ---------------------------------------------------------------- text

export function T({
  children,
  v = 'body',
  style,
  numberOfLines,
}: {
  children: ReactNode;
  v?: keyof typeof type;
  style?: StyleProp<TextStyle>;
  numberOfLines?: number;
}) {
  return (
    <Text style={[type[v], style]} numberOfLines={numberOfLines}>
      {children}
    </Text>
  );
}

export function Brand({ size = 20 }: { size?: number }) {
  return <Text style={{ fontFamily: fonts.brand, fontSize: size, color: colors.text, letterSpacing: -0.2 }}>dawbot</Text>;
}

export function Title({ title, sub, style }: { title: string; sub?: string; style?: StyleProp<ViewStyle> }) {
  return (
    <View style={[{ gap: 8 }, style]}>
      <T v="title" style={{}} >{title}</T>
      {sub ? <T v="sub">{sub}</T> : null}
    </View>
  );
}

// ---------------------------------------------------------------- buttons

type ButtonProps = {
  label: string;
  onPress?: () => void;
  href?: string;
  variant?: 'primary' | 'secondary' | 'ghost' | 'light' | 'danger';
  icon?: LucideIcon;
  style?: StyleProp<ViewStyle>;
  compact?: boolean;
  disabled?: boolean;
};

export function Button({ label, onPress, href, variant = 'primary', icon: Icon, style, compact, disabled }: ButtonProps) {
  const v = buttonVariants[variant];
  return (
    <Pressable
      accessibilityRole="button"
      accessibilityState={{ disabled }}
      disabled={disabled}
      onPress={() => {
        onPress?.();
        if (href) router.push(href as never);
      }}
      style={({ pressed }) => [
        styles.button,
        { height: compact ? 44 : 52, backgroundColor: v.bg, borderColor: v.border, borderWidth: v.border ? 1 : 0, opacity: disabled ? 0.5 : 1 },
        pressed && { transform: [{ scale: 0.98 }], opacity: 0.9 },
        style,
      ]}
    >
      {Icon ? <Icon size={18} color={v.fg} strokeWidth={1.75} /> : null}
      <Text style={{ fontFamily: variant === 'ghost' ? fonts.medium : fonts.semibold, fontSize: compact ? 15 : 16, color: v.fg }}>{label}</Text>
    </Pressable>
  );
}

const buttonVariants = {
  primary: { bg: colors.coral, fg: colors.onCoral, border: undefined },
  secondary: { bg: colors.raised, fg: colors.text, border: colors.line },
  ghost: { bg: 'transparent', fg: colors.text2, border: undefined },
  light: { bg: colors.text, fg: colors.onCoral, border: undefined },
  danger: { bg: colors.red, fg: '#2A1214', border: undefined },
} as const;

export function IconButton({
  icon: Icon,
  label,
  onPress,
  href,
  tone = 'plain',
  size = 22,
}: {
  icon: LucideIcon;
  label: string;
  onPress?: () => void;
  href?: string;
  tone?: 'plain' | 'card' | 'coral';
  size?: number;
}) {
  const bg = tone === 'card' ? colors.card : tone === 'coral' ? colors.coral : 'transparent';
  const fg = tone === 'coral' ? colors.onCoral : colors.text;
  return (
    <Pressable
      accessibilityRole="button"
      accessibilityLabel={label}
      hitSlop={6}
      onPress={() => {
        onPress?.();
        if (href) router.push(href as never);
      }}
      style={({ pressed }) => [styles.iconButton, { backgroundColor: bg }, pressed && { opacity: 0.7 }]}
    >
      <Icon size={size} color={fg} strokeWidth={1.75} />
    </Pressable>
  );
}

// ---------------------------------------------------------------- surfaces

export function Card({ children, style, onPress }: { children: ReactNode; style?: StyleProp<ViewStyle>; onPress?: () => void }) {
  if (onPress) {
    return (
      <Pressable accessibilityRole="button" onPress={onPress} style={({ pressed }) => [styles.card, style, pressed && { opacity: 0.85 }]}>
        {children}
      </Pressable>
    );
  }
  return <View style={[styles.card, style]}>{children}</View>;
}

export function Banner({
  icon: Icon,
  title,
  body,
  tone = 'coral',
  children,
  style,
}: {
  icon: LucideIcon;
  title?: string;
  body: string;
  tone?: 'coral' | 'green' | 'neutral';
  children?: ReactNode;
  style?: StyleProp<ViewStyle>;
}) {
  const t = {
    coral: { bg: colors.coralBg, ic: colors.coralText, fg: colors.coralMuted },
    green: { bg: colors.greenBg, ic: colors.green, fg: '#BFE8D2' },
    neutral: { bg: colors.card, ic: colors.coralText, fg: colors.text2 },
  }[tone];
  return (
    <View style={[{ backgroundColor: t.bg, borderRadius: radius.lg, padding: 14, flexDirection: 'row', gap: 12 }, tone === 'neutral' && { borderWidth: 1, borderColor: colors.line }, style]}>
      <Icon size={20} color={t.ic} strokeWidth={1.75} />
      <View style={{ flex: 1, gap: 2 }}>
        {title ? <T v="bodyStrong" style={{ fontSize: 14 }}>{title}</T> : null}
        <T v="small" style={{ color: t.fg }}>{body}</T>
        {children}
      </View>
    </View>
  );
}

export function Pill({ label, tone = 'neutral', icon: Icon }: { label: string; tone?: 'neutral' | 'coral' | 'green' | 'muted'; icon?: LucideIcon }) {
  const t = {
    neutral: { bg: colors.raised, fg: colors.text2 },
    muted: { bg: colors.raised, fg: colors.text3 },
    coral: { bg: colors.coralBg, fg: colors.coralText },
    green: { bg: colors.greenBg, fg: colors.green },
  }[tone];
  return (
    <View style={{ flexDirection: 'row', alignItems: 'center', gap: 4, height: 24, paddingHorizontal: 10, borderRadius: radius.pill, backgroundColor: t.bg, alignSelf: 'flex-start' }}>
      {Icon ? <Icon size={12} color={t.fg} strokeWidth={2} /> : null}
      <Text style={{ fontFamily: fonts.medium, fontSize: 12, color: t.fg }}>{label}</Text>
    </View>
  );
}

export function Chip({ label, on, onPress, icon: Icon, small }: { label: string; on?: boolean; onPress?: () => void; icon?: LucideIcon; small?: boolean }) {
  return (
    <Pressable
      accessibilityRole="button"
      accessibilityState={{ selected: !!on }}
      onPress={onPress}
      style={({ pressed }) => [
        styles.chip,
        { height: small ? 32 : 40 },
        on && { backgroundColor: colors.coralBg, borderColor: colors.coral },
        pressed && { opacity: 0.8 },
      ]}
    >
      {Icon ? <Icon size={small ? 14 : 18} color={on ? colors.coralText : '#D4D4D8'} strokeWidth={1.75} /> : null}
      <Text style={{ fontFamily: fonts.medium, fontSize: small ? 13 : 14, color: on ? colors.coralText : '#D4D4D8' }}>{label}</Text>
    </Pressable>
  );
}

export function Toggle({ value, onChange, label }: { value: boolean; onChange: (v: boolean) => void; label: string }) {
  return (
    <Pressable
      accessibilityRole="switch"
      accessibilityLabel={label}
      accessibilityState={{ checked: value }}
      onPress={() => onChange(!value)}
      hitSlop={8}
      style={{ width: 44, height: 26, borderRadius: 13, backgroundColor: value ? colors.coral : colors.raised, justifyContent: 'center' }}
    >
      <View style={{ width: 20, height: 20, borderRadius: 10, backgroundColor: value ? '#fff' : '#D4D4D8', marginLeft: value ? 21 : 3 }} />
    </Pressable>
  );
}

export function SelectCard({
  selected,
  onPress,
  children,
  style,
  role = 'radio',
}: {
  selected: boolean;
  onPress: () => void;
  children: ReactNode;
  style?: StyleProp<ViewStyle>;
  role?: 'radio' | 'button';
}) {
  return (
    <Pressable
      accessibilityRole={role}
      accessibilityState={role === 'radio' ? { checked: selected } : { selected }}
      onPress={onPress}
      style={({ pressed }) => [
        styles.selectCard,
        selected ? { borderColor: colors.coral, borderWidth: 1.5, backgroundColor: colors.coralBg } : null,
        pressed && { opacity: 0.85 },
        style,
      ]}
    >
      {children}
    </Pressable>
  );
}

export function Radio({ on }: { on: boolean }) {
  return (
    <View style={{ width: 22, height: 22, borderRadius: 11, borderWidth: 2, borderColor: on ? colors.coral : '#4A4A52', alignItems: 'center', justifyContent: 'center' }}>
      {on ? <View style={{ width: 10, height: 10, borderRadius: 5, backgroundColor: colors.coral }} /> : null}
    </View>
  );
}

export function SectionHeader({ title, right, style }: { title: string; right?: string; style?: StyleProp<ViewStyle> }) {
  return (
    <View style={[{ flexDirection: 'row', justifyContent: 'space-between', alignItems: 'baseline' }, style]}>
      <T v="section">{title}</T>
      {right ? <T v="muted">{right}</T> : null}
    </View>
  );
}

export function IconTile({ icon: Icon, size = 40, color = colors.coralText }: { icon: LucideIcon; size?: number; color?: string }) {
  return (
    <View style={{ width: size, height: size, borderRadius: 12, backgroundColor: colors.raised, alignItems: 'center', justifyContent: 'center' }}>
      <Icon size={size / 2} color={color} strokeWidth={1.75} />
    </View>
  );
}

// ---------------------------------------------------------------- headers

export function NavHeader({ title, close, right, onBack }: { title?: string; close?: boolean; right?: ReactNode; onBack?: () => void }) {
  return (
    <View style={styles.navHeader}>
      <IconButton
        icon={close ? X : ChevronLeft}
        label={close ? 'Close' : 'Back'}
        size={24}
        onPress={onBack ?? (() => (router.canGoBack() ? router.back() : router.replace('/learn' as never)))}
      />
      <Text style={{ flex: 1, textAlign: 'center', fontFamily: fonts.semibold, fontSize: 16, color: colors.text }}>{title}</Text>
      {right ?? <View style={{ width: 44 }} />}
    </View>
  );
}

export function ProgressBar({ value, height = 8, color = colors.coral }: { value: number; height?: number; color?: string }) {
  const pct = Math.max(0, Math.min(1, value));
  return (
    <View
      accessibilityRole="progressbar"
      accessibilityValue={{ min: 0, max: 100, now: Math.round(pct * 100) }}
      style={{ flex: 1, height, borderRadius: height / 2, backgroundColor: colors.raised, overflow: 'hidden' }}
    >
      <View style={{ width: `${pct * 100}%`, height: '100%', borderRadius: height / 2, backgroundColor: color }} />
    </View>
  );
}

export function StepHeader({ value }: { value: number }) {
  return (
    <View style={[styles.navHeader, { paddingRight: 20 }]}>
      <IconButton icon={ChevronLeft} label="Back" size={24} onPress={() => router.back()} />
      <ProgressBar value={value} />
      <View style={{ width: 12 }} />
    </View>
  );
}

export function LessonHeader({ value, streak }: { value: number; streak: number }) {
  return (
    <View style={[styles.navHeader, { paddingRight: 20, gap: 12 }]}>
      <IconButton icon={X} label="Quit lesson" size={24} onPress={() => router.dismissTo('/learn' as never)} />
      <ProgressBar value={value} height={10} />
      <Row gap={4}>
        <Flame size={18} color={colors.coral} fill={colors.coral} />
        <T v="bodyStrong" style={{ color: colors.coral, fontSize: 14 }}>{streak}</T>
      </Row>
    </View>
  );
}

// ---------------------------------------------------------------- music bits

const WHITE_KEYS = ['C', 'D', 'E', 'F', 'G', 'A', 'B'] as const;
const BLACK_KEY_LEFT = [10, 24.3, 52.9, 67.2, 81.5];

export function Piano({ highlight = [], height = 120, onKeyPress }: { highlight?: string[]; height?: number; onKeyPress?: (note: string) => void }) {
  return (
    <View accessibilityLabel={`Piano keys, ${highlight.join(', ') || 'none'} highlighted`} style={{ height, flexDirection: 'row', borderRadius: 10, overflow: 'hidden', backgroundColor: colors.piano }}>
      {WHITE_KEYS.map((k, i) => {
        const on = highlight.includes(k);
        return (
          <Pressable
            key={k}
            accessibilityRole="button"
            accessibilityLabel={`Key ${k}`}
            onPress={() => onKeyPress?.(k)}
            style={{ flex: 1, alignItems: 'center', justifyContent: 'flex-end', paddingBottom: 8, backgroundColor: on ? colors.coral : undefined, borderRightWidth: i === 6 ? 0 : 1, borderRightColor: colors.pianoLine }}
          >
            {on ? <Text style={{ fontFamily: fonts.semibold, fontSize: 12, color: colors.onCoral }}>{k}</Text> : null}
          </Pressable>
        );
      })}
      {BLACK_KEY_LEFT.map((left) => (
        <View key={left} pointerEvents="none" style={{ position: 'absolute', top: 0, left: `${left}%`, width: '8%', height: '58%', backgroundColor: '#141416', borderBottomLeftRadius: 3, borderBottomRightRadius: 3 }} />
      ))}
    </View>
  );
}

function seeded(seed: number) {
  let s = seed * 9301 + 49297;
  return () => {
    s = (s * 9301 + 49297) % 233280;
    return s / 233280;
  };
}

export function Waveform({ bars = 40, seed = 3, played, height = 40, barWidth = 3, gap = 3, color = colors.coral, pump }: { bars?: number; seed?: number; played?: number; height?: number; barWidth?: number; gap?: number; color?: string; pump?: boolean }) {
  const rand = seeded(seed);
  return (
    <View style={{ flexDirection: 'row', alignItems: 'center', gap, height }} accessibilityElementsHidden importantForAccessibility="no-hide-descendants">
      {Array.from({ length: bars }, (_, i) => {
        const v = pump ? 20 + (i % 8) * 10 : 20 + 80 * rand() * (0.6 + (0.4 * (i % 7)) / 7);
        const c = played === undefined || i < played ? color : '#4A4A52';
        return <View key={i} style={{ width: barWidth, height: `${v}%`, borderRadius: 2, backgroundColor: c }} />;
      })}
    </View>
  );
}

export function DawIcon({ id, size = 28 }: { id: DawId; size?: number }) {
  const d = dawById(id);
  if (!d.icon) return null;
  return <Image source={d.icon} style={{ width: size, height: size }} accessibilityIgnoresInvertColors />;
}

export function DawPill({ id = 'ableton', label = 'Ableton Live 12' }: { id?: DawId; label?: string }) {
  return (
    <View style={{ flexDirection: 'row', alignItems: 'center', gap: 8, height: 36, paddingLeft: 6, paddingRight: 12, borderRadius: radius.pill, backgroundColor: colors.card, borderWidth: 1, borderColor: colors.line }}>
      <DawIcon id={id} size={24} />
      <Text style={{ fontFamily: fonts.medium, fontSize: 13, color: colors.text }}>{label}</Text>
    </View>
  );
}

export function CheckMark({ size = 18, color = colors.coral }: { size?: number; color?: string }) {
  return <Check size={size} color={color} strokeWidth={2.25} />;
}

// ---------------------------------------------------------------- styles

const styles = StyleSheet.create({
  screen: { flex: 1, backgroundColor: colors.bg },
  footer: { paddingHorizontal: 20, paddingTop: 12, gap: 8, backgroundColor: colors.bg },
  button: { flexDirection: 'row', alignItems: 'center', justifyContent: 'center', gap: 8, borderRadius: radius.lg, paddingHorizontal: 16 },
  iconButton: { width: 44, height: 44, borderRadius: 12, alignItems: 'center', justifyContent: 'center' },
  card: { backgroundColor: colors.card, borderRadius: radius.xl, borderWidth: 1, borderColor: colors.line },
  chip: { flexDirection: 'row', alignItems: 'center', gap: 8, paddingHorizontal: 14, borderRadius: radius.pill, borderWidth: 1, borderColor: colors.lineStrong },
  selectCard: { borderRadius: radius.lg, borderWidth: 1, borderColor: colors.line, backgroundColor: colors.card },
  navHeader: { flexDirection: 'row', alignItems: 'center', paddingHorizontal: 8, paddingTop: 4, paddingBottom: 8, minHeight: 52 },
});
