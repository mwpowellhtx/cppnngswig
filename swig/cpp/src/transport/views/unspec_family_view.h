#ifndef NNGCPP_TRANSPORT_VIEWS_UNSPEC_FAMILY_VIEW_H
#define NNGCPP_TRANSPORT_VIEWS_UNSPEC_FAMILY_VIEW_H

#include "IFamilyView.h"

namespace nng {

    struct _UnspecFamilyView : public IAddrFamilyView<sockaddr_unspec_t> {

        virtual ~_UnspecFamilyView();

        virtual bool HasOne() const override;

    protected:

        friend void align_view(const _SockAddr&);

        _UnspecFamilyView(sockaddr_type* const sap);

        virtual detail_type* get_detail() const override;

    public:

        virtual bool Equals(const IAddrFamilyViewBase& other) const override;

        virtual bool operator==(const IAddrFamilyViewBase& other) override;
        virtual bool operator!=(const IAddrFamilyViewBase& other) override;

        virtual nng::uint32_t GetIPv4Addr() const override;

        virtual void SetIPv4Addr(const nng::uint32_t value) override;

        virtual IPv6AddrVector GetIPv6Addr() const override;
        virtual IPv6AddrUInt16Vector GetIPv6Addr16() const override;
        virtual IPv6AddrUInt32Vector GetIPv6Addr32() const override;

        virtual void SetIPv6Addr(IPv6AddrVector value) override;
        virtual void SetIPv6Addr16(IPv6AddrUInt16Vector value) override;
        virtual void SetIPv6Addr32(IPv6AddrUInt32Vector value) override;

        virtual nng::uint16_t __GetPort() const override;
        virtual void __SetPort(const nng::uint16_t value) override;
    };
}

#endif // NNGCPP_TRANSPORT_VIEWS_UNSPEC_FAMILY_VIEW_H
