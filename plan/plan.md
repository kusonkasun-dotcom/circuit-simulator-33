# ArduLab — Desktop EDA (Fondasi + Katalog + Kanvas)

Aplikasi desktop offline-first untuk merancang rangkaian elektronika: mengelola katalog komponen, mengimpor definisi komponen dari JSON, dan menempatkannya di kanvas gambar berukuran A3.
Dibangun dari nol dengan C++/Qt6 sebagai aplikasi Windows/desktop (bukan web), dengan arsitektur yang siap tumbuh menjadi EDA lengkap secara bertahap.

## Untuk siapa
Perancang elektronika, hobbyist, dan engineer yang ingin alat desktop yang bekerja penuh tanpa internet. Fitur AI kelak bersifat opsional dan tidak pernah menjadi syarat untuk membuka, mengedit, atau menyimpan proyek.

## Fitur inti dan pengalaman
- **Katalog komponen pribadi** yang tersimpan permanen di komputer pengguna. Katalog tetap ada setelah aplikasi ditutup dan dibuka kembali.
- **Impor komponen dari file JSON** (format kanonik v1.0). Hasil impor masuk sebagai **DRAFT** dan diberi tanda visual jelas. Setiap impor menghasilkan laporan ringkas: berapa yang berhasil, peringatan, dan error.
- **Pencarian & filter komponen** berdasarkan nama, kategori, dan status (DRAFT/VALIDATED) di panel kiri.
- **Kanvas A3 (420 × 297 mm)** dengan grid, zoom, dan pan. Komponen ditarik dari katalog ke kanvas.
- **Manipulasi komponen**: pilih, pindahkan, putar per 90°, hapus, dengan grid snapping. Titik pin komponen mengikuti perpindahan dan rotasi secara benar.
- **Panel properti** untuk reference, value, posisi, dan rotasi komponen terpilih.
- **Undo/redo** untuk penempatan, pemindahan, rotasi, penghapusan, dan perubahan properti.
- **Status bar** menampilkan posisi kursor dalam mm, posisi setelah snapping, dan identitas pin terdekat bila kursor berada dalam toleransi.
- **Proyek `.fal`**: New, Open, Save, Save As, indikator perubahan belum disimpan, dan konfirmasi saat menutup proyek yang berubah. Proyek menyimpan salinan (snapshot) definisi komponen yang dipakai sehingga tetap bisa dibuka meski katalog berubah.

## Alur pengguna
1. Buka ArduLab. Jika katalog belum ada, database dibuat otomatis pada penggunaan pertama.
2. Impor satu atau beberapa komponen dari file JSON → baca laporan hasil impor → komponen muncul di browser sebagai DRAFT.
3. Cari/filter komponen, lihat propertinya, lalu tarik ke kanvas.
4. Atur posisi: pindahkan, putar, snap ke grid; ubah reference/value di panel properti; pantau posisi & pin di status bar.
5. Simpan sebagai proyek `.fal`. Tutup aplikasi. Buka kembali → katalog dan proyek utuh.

## Rasa UI/UX
Tata letak aplikasi desktop klasik dan fungsional: menu + toolbar di atas, component browser di kiri, kanvas di tengah, panel properti di kanan, status bar di bawah. Semua panel dapat diubah ukurannya. Kontrol yang tampil harus benar-benar berfungsi atau ditandai jelas "belum tersedia" — tanpa tombol kosong. Fokus pada keterbacaan dan koordinat yang akurat, bukan gaya dekoratif.

## Fase implementasi

### Fase 1 — MVP (dibangun sekarang)
Mencakup Milestone 1–5 dari spesifikasi:
- Fondasi aplikasi: window utama, layout panel, identitas komponen, koordinat domain milimeter, logging/error dasar.
- Model komponen & schema JSON v1.0 terdokumentasi (pemisahan definisi katalog vs instance proyek) + contoh JSON valid untuk Resistor, Kapasitor, LED, Ground, dan sumber tegangan DC.
- Katalog SQLite per-pengguna: pembuatan database otomatis, migration framework + Migration 001, versi schema, transaksi, repository, simpan/baca/cari/filter.
- JSON Import dengan validasi penuh, impor sebagai DRAFT, deteksi ID duplikat dengan kebijakan **lewati + laporkan**, perlindungan terhadap impor parsial.
- Kanvas A3: grid, zoom, pan, drag-and-drop, select/move/rotate/delete, snapping, panel properti, undo/redo, status bar readout (toleransi pin berbasis jarak layar/piksel).
- Penyimpanan proyek `.fal` (JSON berstruktur) dengan snapshot definisi komponen, penulisan file atomik, dan penanganan file rusak tanpa kehilangan proyek aktif.

Verifikasi Fase 1: kompilasi Linux + pengujian headless (persistence, migrasi idempoten, validasi impor, transformasi pin, snapping termasuk koordinat negatif, save/load). GUI interaktif dan Windows/MSVC ditandai **belum diverifikasi** sampai benar-benar diuji di lingkungan tersebut.

### Fase 2 — Skematik & koneksi (nanti)
Connection Core (wire, junction, net, koneksi pin), schematic editor, dan pemeriksaan aturan listrik berdasarkan tipe pin. Update definisi katalog sebagai tindakan eksplisit (bukan diam-diam).

### Fase 3 — Simulasi, PCB, & AI (nanti)
Backend simulasi terpisah (evaluasi custom MNA dan/atau ngspice, dengan batas kemampuan jelas), PCB layout + keluaran manufaktur, AI Import datasheet→draft komponen, AI Design Assistant, dan Firmware Generator (prioritas Arduino). Semua fitur AI opsional; editor inti tetap jalan offline.

## Asumsi
- Aplikasi dibangun **dari nol**; tidak ada kode "Phase 1" sebelumnya yang dipakai atau dicari lagi.
- Format proyek `.fal` versi awal = dokumen JSON berstruktur dengan versi schema (bukan format biner).
- Kebijakan ID komponen duplikat saat impor: **impor sebagai varian/entri baru** bila ID berbeda; untuk ID yang benar-benar identik, **lewati dan laporkan** (update definisi menyusul sebagai aksi eksplisit di fase berikutnya).
- Komponen DRAFT **boleh** langsung ditempatkan ke kanvas, dengan penanda visual DRAFT.
- Toleransi snap ke pin di status bar berbasis **jarak layar (piksel)** agar konsisten di semua tingkat zoom.
- Dukungan format JSON legacy **tidak** dibuat sekarang (tidak ada contoh nyata); hanya disiapkan batas modul agar adapter legacy bisa ditambahkan nanti.
- Contoh JSON komponen dan satu contoh proyek `.fal` disertakan sebagai bagian hasil.
- Verifikasi di lingkungan ini terbatas pada build Linux + unit test headless (mis. Qt offscreen); screenshot offscreen (bila platform tersedia) hanya untuk cek layout, bukan pengganti uji GUI interaktif.
- Katalog disimpan di lokasi data pengguna standar OS (via mekanisme path aplikasi Qt), bukan di direktori instalasi.
- Sumber kode desktop ditempatkan di direktori `ArduLab/` terpisah; scaffold web default yang ada tidak dijadikan bagian aplikasi.
