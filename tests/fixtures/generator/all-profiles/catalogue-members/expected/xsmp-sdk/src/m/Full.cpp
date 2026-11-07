// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Full.cpp
// -----------------------------------------------------------------------------
/// @file m/Full.cpp

#include <m/Full.h>
// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/IBase.h>

#include <stdexcept>

namespace m
{
void Full::DoPublish(::Smp::IPublication* receiver) {
}

void Full::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Full::DoConnect(::Smp::ISimulator* simulator) {
}

void Full::DoDisconnect() {
}

void Full::_main() {
    // TODO implement EntryPoint main
    /// Run it.
}

void Full::_second() {
    // TODO implement EntryPoint second
}

void Full::_onTick(::Smp::IObject* sender, ::Smp::Int32) {
    // TODO implement EventSink onTick
    /// Sink.
}

void Full::_onBang(::Smp::IObject* sender) {
    // TODO implement EventSink onBang
}

::Smp::Int32 Full::mustImplement() {
    // TODO
    return 0;
}

void Full::other() {
    // TODO
}

void Full::noArgs() {
    // TODO
}

::Smp::Int32 Full::retInt() {
    // TODO
    return 0;
}

::Smp::Bool Full::retBool() {
    // TODO
    return false;
}

::m::Mode Full::retEnum() {
    // TODO
    return ::m::Mode::Off;
}

::m::Name Full::retName() {
    // TODO
    return "";
}

::m::Pair Full::retStruct() {
    // TODO
    return {/*.a = */0, /*.b = */0};
}

::m::Three Full::retArray() {
    // TODO
    return {};
}

::m::IBase* Full::retIface() {
    // TODO
    return nullptr;
}

::Smp::String8 Full::retStr8() {
    // TODO
    return "";
}

::Smp::Bool Full::retChar() {
    // TODO
    return false;
}

::Smp::Float32 Full::retF32() {
    // TODO
    return 0.0f;
}

::Smp::UInt64 Full::retU64() {
    // TODO
    return 0UL;
}

::Smp::Duration Full::retDuration() {
    // TODO
    return 0L;
}

void Full::withIn(::Smp::Int32 a, ::Smp::Float64 b, ::Smp::Bool c) {
    // TODO
}

void Full::withOut(::Smp::Int32* a, ::Smp::Float64* b) {
    // TODO
}

void Full::withStr8(::Smp::String8 s) {
    // TODO
}

void Full::withStr8Out(::Smp::String8* s) {
    // TODO
}

void Full::withName(::m::Name n, ::m::Name* o) {
    // TODO
}

void Full::withDefaultOut(::Smp::Int32* a) {
    // TODO
}

void Full::withEnum(::m::Mode a) {
    // TODO
}

void Full::withStruct(::m::Pair p) {
    // TODO
}

void Full::withIface(const ::m::IBase& i) {
    // TODO
}

void Full::withRef(::Smp::Int32& i) {
    // TODO
}

void Full::withPtr(::Smp::Int32* i) {
    // TODO
}

void Full::withConst(const ::Smp::Int32 i) {
    // TODO
}

::Smp::Int32 FullGen::statOp(::Smp::Int32 a) {
    // TODO
    return 0;
}

::Smp::Int32 FullGen::nonVirt(::Smp::Int32 a) {
    // TODO
    return 0;
}

::Smp::Int32 Full::constOp() const {
    // TODO
    return 0;
}

::Smp::Int32 Full::viewed(::Smp::Int32 a) {
    // TODO
    return 0;
}

void Full::viewedVoid() {
    // TODO
}

::Smp::Int32 Full::debugged(::m::Name a) {
    // TODO
    return 0;
}

void Full::viewedOut(::Smp::Int32* a) {
    // TODO
}

void Full::operator+=(::Smp::Int32 a) {
    // TODO
}

void Full::raises(::Smp::Int32 a) {
    // TODO
}

::Smp::Int32 Full::get_plainProp(){
    ::Smp::Int32 ret {};
    return ret;
}
void Full::set_plainProp(::Smp::Int32 value){
    // TODO
}

::Smp::Int32 Full::get_roProp(){
    ::Smp::Int32 ret {};
    return ret;
}

void Full::set_woProp(::Smp::Int32 value){
    // TODO
}

::Smp::Int32 Full::get_rwProp(){
    ::Smp::Int32 ret {};
    return ret;
}
void Full::set_rwProp(::Smp::Int32 value){
    // TODO
}

::m::Name Full::get_nameProp(){
    ::m::Name ret {};
    return ret;
}
void Full::set_nameProp(::m::Name value){
    // TODO
}

::m::Pair Full::get_pairProp(){
    ::m::Pair ret {};
    return ret;
}
void Full::set_pairProp(::m::Pair value){
    // TODO
}

::m::Mode Full::get_modeProp(){
    ::m::Mode ret {};
    return ret;
}
void Full::set_modeProp(::m::Mode value){
    // TODO
}

::Smp::Int32 Full::get_constGet() const{
    ::Smp::Int32 ret {};
    return ret;
}
void Full::set_constGet(::Smp::Int32 value){
    // TODO
}

::Smp::Int32 Full::get_protProp(){
    ::Smp::Int32 ret {};
    return ret;
}
void Full::set_protProp(::Smp::Int32 value){
    // TODO
}

::Smp::Int32 Full::get_privProp(){
    ::Smp::Int32 ret {};
    return ret;
}
void Full::set_privProp(::Smp::Int32 value){
    // TODO
}

::m::IBase* Full::get_ifaceProp(){
    ::m::IBase* ret {};
    return ret;
}
void Full::set_ifaceProp(::m::IBase* value){
    // TODO
}

::Smp::Int32* Full::get_ptrProp(){
    ::Smp::Int32* ret {};
    return ret;
}
void Full::set_ptrProp(::Smp::Int32* value){
    // TODO
}

::Smp::Int32& Full::get_refProp(){
    throw std::logic_error("Property getter is not implemented.");
}
void Full::set_refProp(::Smp::Int32& value){
    // TODO
}

const ::Smp::Int32 Full::get_constProp(){
    const ::Smp::Int32 ret {};
    return ret;
}
void Full::set_constProp(const ::Smp::Int32 value){
    // TODO
}
} // namespace m
