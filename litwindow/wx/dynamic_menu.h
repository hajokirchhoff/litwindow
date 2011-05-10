#ifndef dynamic_menu_h__301108
#define dynamic_menu_h__301108

#include <vector>
#include <utility>

namespace litwindow { namespace wx {
    template <class Value>
    struct dynamic_menu_value_traits
    {
        bool checked(const Value &) const { return false; }
        bool checkable(const Value &) const { return false; }
        wxString helptext(const Value &) const { return wxEmptyString; }
    };

    template <class Value, class Traits=dynamic_menu_value_traits<typename Value> >
    class dynamic_menu
    {
    public:
        void append(const Value &v, const wxString &label)
        {
            m_values.push_back(make_pair(v, label));
            add_to_menu(m_values.size()-1);
        }
		void set(size_t idx, const Value &v, const wxString &label)
		{
			m_values.at(idx)=make_pair(v, label);
			add_to_menu(idx);
		}
		void resize(size_t new_size)
		{
			// first remove no longer used entries
			size_t unused_entries=new_size;
			while (unused_entries<m_values.size()) {
				remove_menu(unused_entries);
				++unused_entries;
			}
			// now add entries if neccessary
			size_t i=m_values.size();
			m_values.resize(new_size);
			// and add entries in the menu itself
			while (i<m_values.size()) {
				set(i++, Value(), L"???");
			}
		}
        void remove(const Value &v, const wxString &label=wxEmptyString)
        {
            size_t target=0;
            for (size_t i=0; i<m_values.size(); ++i) {
                if (m_values[i].first!=v || label.empty()==false && label!=m_values[i].second) {
                    if (i!=target) {
                        m_values[target]=m_values[i];
                        set_menu(target);
                        ++target;
                    }
                }
                ++i;
            }
            size_t new_size=target;
            while (target<m_values.size()) {
                remove_menu(target);
                ++target;
            }
            m_values.resize(new_size);
        }
        const Value &get_value(int menu_id) const
        {
            return m_values.at(menu_id-m_start_id).first;
        }
        void use_menu(wxMenu *m, int start_id, int insert_before_id=0)
        {
            m_menu=m;
            m_start_id=start_id;
            m_insert_before_id=insert_before_id;
        }
        wxMenu *get_menu() const { return m_menu; }
        dynamic_menu(const Traits &t=Traits()):m_traits(t) {}
        void update_menu()
        {
            if (m_menu) {
                for (size_t i=0; i<m_values.size(); ++i) {
                    set_menu(i);
                }
            }
        }
    protected:
        void add_to_menu(size_t index)
        {
            if (m_menu) {
                wxMenuItem *item=m_menu->FindItem(m_start_id+(int)index);
                if (item==0) {
                    // no item yet, need to create one
                    size_t insert_position=0;
                    if (m_insert_before_id!=0) {
                        // try to find the insert position
                        wxMenuItem *last_found;
                        while (insert_position<m_menu->GetMenuItemCount() && (last_found=m_menu->FindItemByPosition(insert_position))->GetId()!=m_insert_before_id) {
                            ++insert_position;
                        }
                        if (insert_position<m_menu->GetMenuItemCount()) {
                            while (insert_position>0 && m_menu->FindItemByPosition(insert_position-1)->IsSeparator())
                                --insert_position;
                        } else {
                            m_menu->AppendSeparator();
                            ++insert_position;
                        }
                    } else
                        insert_position=m_menu->GetMenuItemCount();
                    item=m_menu->Insert(insert_position, m_start_id+(int)index, m_values[index].second);
                }
                set_menu(index, item);
            }
        }
        void set_menu( size_t index, wxMenuItem * item )
        {
            item->SetItemLabel(m_values[index].second);
            const Value &v(m_values[index].first);
            bool checkable=m_traits.checkable(v);
            item->SetCheckable(checkable);
            if (checkable)
                item->Check(m_traits.checked(v));
            item->SetHelp(m_traits.helptext(v));
        }
        void set_menu(size_t index)
        {
            if (m_menu) {
                wxMenuItem *item=m_menu->FindItem(m_start_id+(int)index);
                set_menu(index, item);
            }
        }
        void remove_menu(size_t index)
        {
            if (m_menu) {
                m_menu->Destroy(m_start_id+(int)index);
            }
        }
        std::vector<std::pair<Value, wxString> > m_values;
        wxMenu *m_menu;
        int m_start_id;
        int m_insert_before_id;
        Traits m_traits;
    };
}
}

#endif // dynamic_menu_h__301108

#ifdef _MSC_VER
#pragma once
#endif
